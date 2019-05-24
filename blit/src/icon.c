#define XSIZE 50
#define YSIZE 50

#define bool  int
#define true  1
#define false 0

#define NOHIT    -1

#define I_NULL   -1
#define I_OR      0
#define I_STORE   1
#define I_CLR     2
#define I_XOR     3

#include <jerq.h>
#include <jerqio.h>
#include <font.h>

#define SPACING 24

#define sgn(x) ((x)<0 ? -1 : (x)==0 ? 0 : 1)

int horsize(r)
Rectangle r;
{
   return(r.corner.x - r.origin.x);
}

int versize(r)
Rectangle r;
{
   return(r.corner.y - r.origin.y);
}

extern Texture *(imenu[]); /* ***** */

int lasthitx = 0, lasthity = 0;
#define SRC_ID     0
#define SRC_CLR    1
char *blitsrctext[] = 
   {"src := src","src := 0",NULL};

#define DST_STORE  0
#define DST_OR     1
#define DST_XOR    2
#define DST_CLR    3
char *blitdsttext[] = 
   {"dst := src","dst := src or dst","dst := src xor dst","dst := 0",NULL};

Menu blitsrcmenu = {blitsrctext};
Menu blitdstmenu = {blitdsttext};

Texture Cmove = {
	 0x0000, 0x0000, 0x00C0, 0x00E0,
	 0x00F0, 0x7FF8, 0x7FDC, 0x600E,
	 0x6007, 0x600E, 0x7FDC, 0x7FF8,
	 0x00F0, 0x00E0, 0x00C0, 0x0000,
};

Texture Ccopy = {
	 0x0000, 0x0000, 0x0000, 0x0040,
	 0x0020, 0x0010, 0x0008, 0x1FFE,
	 0x1023, 0x7623, 0xF023, 0x1022,
	 0x1FFE, 0x0000, 0x0000, 0x0000,
};

Texture Cerase = {
	 0x24C0, 0x00A8, 0x085C, 0x422E,
	 0x0014, 0x27FA, 0x0405, 0x06AF,
	 0x02A8, 0x02A8, 0x02A8, 0x02A8,
	 0x02A8, 0x0EA8, 0x3FA8, 0x7FF8,
};

Texture Cinvert = {
	 0x0000, 0x0000, 0x07C0, 0x07C0,
	 0x07C0, 0x07C0, 0x07C0, 0x783C,
	 0x783C, 0x783C, 0x07C0, 0x07C0,
	 0x07C0, 0x07C0, 0x07C0, 0x0000,
};

Texture Cblit = {
	 0x0000, 0xFE7F, 0x8255, 0xFE7F,
	 0x8295, 0xE0CF, 0x8FA5, 0xE817,
	 0x8815, 0xEFA7, 0x80C5, 0xFE9F,
	 0x8255, 0xFE7F, 0x8255, 0xFE7F,
};

Texture Creflx = {
	 0x0000, 0x0000, 0x7FFE, 0x8001,
	 0x8011, 0xC033, 0xBC5D, 0x8781,
	 0x8501, 0x8501, 0x4482, 0x3C5C,
	 0x0030, 0x0010, 0x0000, 0x0000,
};

Texture Crefly = {
	 0x03F8, 0x0424, 0x0844, 0x0844,
	 0x0844, 0x0FC4, 0x0084, 0x0384,
	 0x0484, 0x0844, 0x1024, 0x3874,
	 0x0844, 0x0844, 0x0424, 0x03F8,
};

Texture Crotplus = {
	 0x0000, 0x0000, 0x3FE0, 0x2010,
	 0x2008, 0x2008, 0x2008, 0x3F08,
	 0x0108, 0x0108, 0x070E, 0x0204,
	 0x0108, 0x0090, 0x0060, 0x0000,
};

Texture Crotminus = {
	 0x0000, 0x0400, 0x0C00, 0x17F0,
	 0x2008, 0x4004, 0x4004, 0x2004,
	 0x1784, 0x0C84, 0x0484, 0x0084,
	 0x0084, 0x00FC, 0x0000, 0x0000,
};

Texture Cshearx = {
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x03FF, 0x0000, 0x0FFC,
	 0x0000, 0x3FF0, 0x0000, 0xFFC0,
	 0x0000, 0x0000, 0x0000, 0x0000,
};

Texture Csheary = {
	 0x0800, 0x0800, 0x0A00, 0x0A00,
	 0x0A80, 0x0A80, 0x0AA0, 0x0AA0,
	 0x0AA0, 0x0AA0, 0x02A0, 0x02A0,
	 0x00A0, 0x00A0, 0x0020, 0x0020,
};

Texture Cstretch = {
	 0x0000, 0x7CAA, 0x7CAA, 0x7CAA,
	 0x7CAA, 0x7CAA, 0x0000, 0x0000,
	 0x7CAA, 0x0000, 0x7CAA, 0x0000,
	 0x7CAA, 0x0000, 0x7CAA, 0x0000,
};

Texture Ctexture = {
	 0x4000, 0x7000, 0xE000, 0x2000,
	 0x0444, 0x0777, 0x0EEE, 0x0222,
	 0x0444, 0x0777, 0x0EEE, 0x0222,
	 0x0444, 0x0777, 0x0EEE, 0x0222,
};

Texture Cgrid = {
	 0x4040, 0xFFFF, 0x4040, 0x4444,
	 0x4040, 0x5555, 0x4040, 0x4444,
	 0x4040, 0xFFFF, 0x4040, 0x4444,
	 0x4040, 0x5555, 0x4040, 0x4444,
};

Texture Ccursor = {
	 0x0000, 0x0000, 0x03E0, 0x17F0,
	 0x3FF0, 0x5FFE, 0xFFF1, 0x0421,
	 0x0002, 0x00FC, 0x0100, 0x0080,
	 0x0040, 0x0080, 0x0000, 0x0000,
};

Texture Cread = {
	 0x0000, 0x0FFE, 0x1FFA, 0x1811,
	 0x0021, 0x8021, 0xC061, 0xC1F1,
	 0x622A, 0x3414, 0x1810, 0x0810,
	 0x0420, 0x03C0, 0x0000, 0x0000,
};

Texture Cwrite = {
	 0xF000, 0xFC00, 0x6B00, 0x3580,
	 0x12C0, 0x1960, 0x0CA0, 0x06B0,
	 0x0250, 0x0358, 0x01A8, 0x00D8,
	 0x0074, 0x101C, 0xBB06, 0xEEFB,
};

Texture Cexit = {
	 0x0001, 0x0002, 0x0002, 0x0002,
	 0x6019, 0x3FFD, 0x3106, 0x31FC,
	 0x3FE0, 0x7D40, 0x5440, 0x6F80,
	 0x5400, 0x6C00, 0x3800, 0x0000,
};

Texture Chelp = {
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0xE0E0, 0x6060, 0x7F7E, 0x7DFF,
	 0x6FFB, 0x6C7B, 0x6FFE, 0xFFF8,
	 0x003C, 0x0000, 0x0000, 0x0000,
};

Texture white = {
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
};

Texture menucursor = {
	 0xFFC0, 0x8040, 0x8040, 0x8040,
	 0xFFC0, 0xFFC0, 0xFE00, 0xFEF0,
	 0x80E0, 0x80F0, 0x80B8, 0xFE1C,
	 0x800E, 0x8047, 0x8042, 0xFFC0,
};

Texture sweepcursor = {
	 0x43FF, 0xE001, 0x7001, 0x3801,
	 0x1D01, 0x0F01, 0x8701, 0x8F01,
	 0x8001, 0x8001, 0x8001, 0x8001,
	 0x8001, 0x8001, 0x8001, 0xFFFF,
};

Texture sweeportrack = {
	 0x43FF, 0xE001, 0x70FD, 0x3805,
	 0x1D05, 0x0F05, 0x8705, 0x8F05,
	 0xA005, 0xA005, 0xA005, 0xA005,
	 0xA005, 0xBFFD, 0x8001, 0xFFFF,
};

Texture clock = {
	 0x03C0, 0x3420, 0x37E0, 0x13C0,
	 0x17F0, 0x1828, 0x2054, 0x20D4,
	 0x418A, 0x430A, 0x430A, 0x418A,
	 0x2094, 0x201C, 0x787E, 0x67F6,
};

Texture deadmouse = {
	 0x0000, 0x0000, 0x0008, 0x0004,
	 0x0082, 0x0441, 0xFFE1, 0x5FF1,
	 0x3FFE, 0x17F0, 0x03E0, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
};

#define MOVE        0
#define COPY        1
#define INVERT	    2
#define ERASE	    3

#define REFLECTX    5
#define REFLECTY    6
#define ROTATEPLUS  7
#define ROTATEMINUS 8

#define SHEARX      10
#define SHEARY      11
#define STRETCH     12
#define TEXTURE     13

#define READ	    15
#define GRID        16
#define PICK        17
#define WRITE	    18

#define BLIT        20
#define SPARE1      21
#define HELP        22
#define EXIT	    23

Texture *(imenu[]) =
   {&Cmove,&Ccopy,&Cinvert,&Cerase,0,
    &Creflx,&Crefly,&Crotplus,&Crotminus,0,
    &Cshearx,&Csheary,&Cstretch,&Ctexture,0,
    &Cread,&Cgrid,&Ccursor,&Cwrite,0,
    &Cblit,&white,&Chelp,&Cexit,0,
    0,0,0,0
   };


char exa[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
char buf[100],FNAME[50];
Point nullpoint, point16x16;
Rectangle icon, ICON, sweep, outl, nullrect, rect16x16;
int Xsize,Ysize;
int Xblocks,Yblocks;
int modx,divx,mody,divy;

flipring(p)
Point p;
{
   outline(&display,raddp(Rect(1,1,SPACING-2,SPACING-2),p));
   outline(&display,raddp(Rect(2,2,SPACING-3,SPACING-3),p));
}

drawimenu(/* imenu, */ xicons,yicons,r)
/* Texture *(imenu[]); */
Rectangle r;
{
   Bitmap *textr;
   int i,j;
   rectf(&display,r,F_CLR);
   outline(&display,Rpt(r.origin,sub(r.corner,Pt(1,1))));
   textr = balloc(Rect(0,0,16,16));
   if (textr == ((Bitmap *) 0)) return(0);
   for (j=0; j<yicons; j++) {
     for (i=0; i<xicons; i++) {
       texture(textr,textr->rect,imenu[j*(xicons+1)+i],F_STORE);
       bitblt(textr,textr->rect,
              &display,
              add(r.origin,Pt(i*SPACING+(SPACING-16)/2,j*SPACING+(SPACING-16)/2)),
              F_OR);
     }
   }
   bfree(textr);
}

Point imenuhit(/* imenu */)
/* Texture *(imenu[]); */
{
   Bitmap *offscreen;
   Point ms,lastms,valley,diff,result,menudrift;
   Rectangle menurect;
   Texture *oldcursor;
   int i,j,hitx,hity;
   int xicons,yicons;
   oldcursor = cursswitch(&white);
   xicons = 0;
   for (j=0; imenu[xicons*j]; j++) {
     for (i=0; imenu[xicons*j+i]; i++);
     i++;
     xicons = i;
   }
   xicons -= 1;
   yicons = j;
   while (!button3()) wait(MOUSE);
   ms = mouse.xy;
   menurect = raddp(Rect(0,0,SPACING*xicons,SPACING*yicons),ms);
   menurect = rsubp(menurect,Pt(SPACING*lasthitx,SPACING*lasthity));
   menudrift.x = min(menurect.corner.x,Drect.corner.x);
   menudrift.y = min(menurect.corner.y,Drect.corner.y);
   menudrift = sub(menudrift,menurect.corner);
   menurect = raddp(menurect,menudrift); cursset(ms=add(ms,menudrift));
   menudrift.x = max(menurect.origin.x,Drect.origin.x);
   menudrift.y = max(menurect.origin.y,Drect.origin.y);
   menudrift = sub(menudrift,menurect.origin);
   menurect = raddp(menurect,menudrift); cursset(add(ms,menudrift));
   offscreen = balloc(menurect);
   if (offscreen != ((Bitmap *) 0)) {
     bitblt(&display,menurect,offscreen,offscreen->rect.origin,F_STORE);
   }
   drawimenu(/* imenu, */ xicons,yicons,menurect);
   lastms = mouse.xy;
   flipring(lastms);
   hitx = hity -1;
   while (button3()) {
     nap(1);
     ms = mouse.xy;
     if (ptinrect(add(ms,Pt(SPACING/2,SPACING/2)),menurect)) {
       lasthitx = hitx = (ms.x-menurect.origin.x+SPACING/2)/SPACING;
       lasthity = hity = (ms.y-menurect.origin.y+SPACING/2)/SPACING;
     } else hitx = hity = -1;
     if (eqpt(lastms,ms) && (hitx != -1)) {
       valley = add(menurect.origin,Pt(SPACING*hitx,SPACING*hity));
       diff = sub(valley,ms);
       ms.x += sgn(diff.x); 
       ms.y += sgn(diff.y); 
       cursset(ms);
     } 
     if (!eqpt(lastms,ms)) {
       flipring(lastms);
       flipring(ms);
     }
     lastms = ms;
   }
   flipring(ms);
   if (offscreen != ((Bitmap *) 0)) {
     bitblt(offscreen,offscreen->rect,&display,menurect.origin,F_STORE);
     bfree(offscreen);
   }
   result.x = hitx;
   result.y = hity;
   cursswitch(oldcursor);
   return(result);
}

Rectangle canonrect(p1, p2)
Point p1, p2;
{
   Rectangle r;
   r.origin.x = min(p1.x, p2.x);
   r.origin.y = min(p1.y, p2.y);
   r.corner.x = max(p1.x, p2.x);
   r.corner.y = max(p1.y, p2.y);
   return(r);
}

Point size(r)
Rectangle r;
{
   Point p;
   p.x = horsize(r);
   p.y = versize(r);
   return(p);
}

int pttopt(p,q)
/* manhattan topology distance between two points */
Point p,q;
{
   return(abs(p.x-q.x)+abs(p.y-q.y));
}

Point nearestcorner(r,p)
Rectangle r;
Point p;
{
   int mindist,dist;
   Point minq,q;
   q = r.origin;
   mindist = pttopt(p,q); minq = q; 
   q.x = r.origin.x; q.y = r.corner.y;
   if ((dist = pttopt(p,q))<mindist) {mindist = dist; minq = q;}
   q = r.corner;
   if ((dist = pttopt(p,q))<mindist) {mindist = dist; minq = q;}
   q.x = r.corner.x; q.y = r.origin.y;
   if ((dist = pttopt(p,q))<mindist) {mindist = dist; minq = q;}
   return(minq);
}

outline(b,r)
Bitmap *b;
Rectangle  r;
{
   segment(b,r.origin,Pt(r.origin.x,r.corner.y),F_XOR);
   segment(b,Pt(r.origin.x,r.corner.y),r.corner,F_XOR);
   segment(b,r.corner,Pt(r.corner.x,r.origin.y),F_XOR);
   segment(b,Pt(r.corner.x,r.origin.y),r.origin,F_XOR);
}


border(r)
  Rectangle r;
{
  outline(&display,inset(r,1));
  outline(&display,inset(r,-1));
}

Rectangle sweeprect()
{
   Rectangle r;
   Point p1, p2;
   Texture *oldcursor;
   oldcursor = cursswitch(&sweepcursor);
   while (!button123()) wait(MOUSE);
   p1=mouse.xy;
   p2=p1;
   r=canonrect(p1, p2);
   outline(&display,r);
   for(; button3(); nap(2)){
     outline(&display,r);
     p2=mouse.xy;
     r=canonrect(p1, p2);
     outline(&display,r);
   }
   outline(&display,r);
   cursswitch(oldcursor);
   return r;
}

bool getr(cliprect,sweep)
Rectangle cliprect;
Rectangle *sweep;
{
   int j;
   *sweep = sweeprect();
   return(rectclip(sweep,cliprect));
}

Point IconPoint(p)
/* convert screen coord to icon coord */
Point p;
{
  p.x = (p.x+Xsize-modx)/Xsize - divx - 1;
  p.y = (p.y+Ysize-mody)/Ysize - divy - 1;
  return(p);
}

Point ScreenPoint(p)
/* convert icon coord to screen coord */
Point p;
{
   p.x = p.x*Xsize + ICON.origin.x;
   p.y = p.y*Ysize + ICON.origin.y;
   return(p);
}

Rectangle IconRect(r)
/* convert a screen rectangle to the biggest totally contained icon rectangle */
Rectangle r;
{
   r.origin = IconPoint(add(r.origin,Pt(Xsize-1,Ysize-1)));
   r.corner = IconPoint(r.corner);
   return(r);
}

Rectangle TrackBorder(r,buttonup)
/* tracks a rectangle "r" (in icon coords) and returns it as it is
   at the end of tracking, clipped to the icon */
Rectangle r;
bool buttonup;
{
   Point newP, oldP;
   Texture *oldcursor;
   oldcursor = cursswitch(&white);
   outl.origin = nullpoint;
   outl.corner.x = horsize(r)*Xsize;
   outl.corner.y = versize(r)*Ysize;
   newP = ScreenPoint(r.origin);
   while (buttonup?(button123()):(!button123())) ;
   cursset(newP);
   oldP = newP;
   border(raddp(outl,newP));
   while (buttonup?(!button123()):(button123())) {
     if (buttonup) wait(MOUSE);
     newP = ScreenPoint(IconPoint(mouse.xy));
     if (!eqpt(newP,oldP)) {
       border(raddp(outl,oldP));
       border(raddp(outl,newP));
       oldP = newP;
     }
   }
   border(raddp(outl,newP));
   cursswitch(oldcursor);
   return(IconRect(raddp(outl,newP)));
}

bool SweepIconRect(r)
Rectangle *r;
{
   Point p;
   if (!getr(ICON,&sweep)) return(false);
   *r = IconRect(sweep);
   return(true);
}

int GetIconRect(r)
/* returns: 0 if no rectangle is provided;
            2 if button2 is used (tracking a 16x16 rectangle);
            3 if button3 is used (sweeping a rectangle);
   if a rectangle is provided, r returns it in icon coordinated */       
Rectangle *r;
{
   int result;
   Texture *oldcursor;
   oldcursor = cursswitch(&sweeportrack);
   while (!button123()) wait(MOUSE);
   if (button3()) {
     if (SweepIconRect(r)) result = 3; 
     else result = 0;
   } else if (button2()) {
     *r = TrackBorder(raddp(rect16x16,add(IconPoint(mouse.xy),Pt(1,1))),false);
     result = 2;
   } else {while(button1()); result = 0;}
   cursswitch(oldcursor);
   return(result);
}

bool GetIconPoint(p)
Point *p;
{
   Rectangle r;
   r = TrackBorder(raddp(Rect(0,0,1,1),IconPoint(mouse.xy)),true);
   while (button123());
   *p = r.origin;
   return((horsize(r) == 1) && (versize(r) == 1));
}

Bitmap *bittester;

bool bitmapbit(b,p)
Bitmap *b;
Point p;
{
   register bit;
   register Word *wp;
   if (!(ptinrect(p,b->rect))) return(false);
   wp = addr(b,p);
   bit = 1<<(WORDSIZE-1)-(p.x&WORDMASK);
   return((*wp&bit)!=0);
}
/*
bool bitmapbit(b,p)
Bitmap *b;
Point p;
{
   if (!(ptinrect(p,b->rect))) return(false);
   *(bittester->base) = 0;
   bitblt(b,Rpt(p,add(p,Pt(1,1))),bittester,Pt(0,0),F_XOR);
   return((*(bittester->base))!=0);
}
*/

char getnibble(b,p)
Bitmap *b;
Point p;
{
   int nibble;
   nibble = 8*bitmapbit(b,p) + 4*bitmapbit(b,Pt(p.x+1,p.y)) +
            2*bitmapbit(b,Pt(p.x+2,p.y)) + bitmapbit(b,Pt(p.x+3,p.y));
   return(exa[nibble]);
}

putnibble(ch,b,clipr,p)
int ch;
Bitmap *b;
Rectangle clipr;
Point p;
{
   int nibble, mask;
   if (ch<'0') return;
   if (ch<='9') nibble = ch - '0';
   else if (ch<='F') nibble = 10 + (ch - 'A');
   else nibble = 10 + (ch - 'a');
   if (nibble != 0)
     for (mask = 0x10; mask >>= 1; p.x++)
       if ((mask & nibble) && ptinrect(p, clipr))
	  point(b, p, F_OR);
}

bool geticonpoint(p)
Point p;
{
   p = add(icon.origin,p);
   if (!ptinrect(p,icon)) return(false);
   return(bitmapbit(&display,p));
}

flipiconpoint(p)
Point p;
{
   Point iconp;
   iconp = add(icon.origin,p);
   if (!ptinrect(iconp,icon)) return(0);
   point(&display,iconp,F_XOR);
   rectf(&display,Rpt(add(Pt(p.x*Xsize,p.y*Ysize),ICON.origin),
                      add(Pt((p.x+1)*Xsize,(p.y+1)*Ysize),ICON.origin)),
         F_XOR);
}

IconOp(bit,p,op)
bool bit;
Point p;
int op;
{
   if ((p.x>=0) && (p.x<Xblocks) && (p.y>=0) && (p.y<Yblocks))
   switch (op) {
     case I_STORE:
       if (geticonpoint(p) != bit) flipiconpoint(p);
       break;
     case I_CLR:
       if (geticonpoint(p) == true) flipiconpoint(p);
       break;
     case I_OR:
       if ((bit == true) && (geticonpoint(p) == false)) flipiconpoint(p);
       break;
     case I_XOR:
       if (bit == true) flipiconpoint(p);
       break;
   }
}

IconBitBlit(from,to,clip,srccode,dstcode)
Rectangle from;  /* icon coords */
Point to;        /* icon coords */
Rectangle clip;  /* icon coords */
int srccode,dstcode;
{
   Rectangle region;
   int dx,dy,di,dj,i,j;
   int left,up;
   bool bit;
   dx = to.x - from.origin.x;
   dy = to.y - from.origin.y;
   left = dx<0;
   up = dy<0;
   di = (left?1:-1);
   dj = (up?1:-1);
   region = from;
   if (!rectclip(&region,raddp(clip,sub(from.origin,to))))
     region = nullrect;
   for (j=(up?from.origin.y:from.corner.y-1);
        up?(j<from.corner.y):(j>=from.origin.y); j+=dj)
     for (i=(left?from.origin.x:from.corner.x-1);
          left?(i<from.corner.x):(i>=from.origin.x); i+=di) {
        bit = geticonpoint(Pt(i,j));
        IconOp(false,Pt(i,j),srccode);
        if (ptinrect(Pt(i,j),region)) IconOp(bit,Pt(i+dx,j+dy),dstcode);
     }
}

horshear(b,r,dx,top)
Bitmap *b;
Rectangle r;
int dx;
bool top;
{
   int i,j,hsize,vsize,shift;
   bool bit,dir;
   hsize = horsize(r);
   vsize = versize(r);
   dir = (dx>0);
/* replaced by a pixel by pixel move because of a bug in bitblt!
   for (j=0; j<vsize; j++) {
     shift = top ? vsize-j-1 : j;
     bitblt(b,Rect(r.origin.x,r.origin.y+j,r.corner.x,r.origin.y+j+1),
             b,Pt(r.origin.x+muldiv(shift,dx,vsize),r.origin.y+j),
             F_STORE);
   }
*/
   for (j=0; j<vsize; j++) {
     shift = top ? vsize-j-1 : j;
     shift = muldiv(shift,dx,vsize);
     for (i=(dir?hsize-1:0); (dir?i>=0:i<hsize); (dir?i--:i++)) {
       bit = bitmapbit(b,Pt(r.origin.x+i,r.origin.y+j));
       if (bit) point(b,Pt(r.origin.x+i+shift,r.origin.y+j),F_STORE);
       else point(b,Pt(r.origin.x+i+shift,r.origin.y+j),F_CLR);
     }
   }
}

vershear(b,r,dy,lft)
Bitmap *b;
Rectangle r;
int dy;
bool lft;
{
   int i,j,hsize,vsize,shift;
   bool bit,dir;
   hsize = horsize(r);
   vsize = versize(r);
   dir = (dy>0);
/* replaced by a pixel by pixel move because of a bug in bitblt!
   for (i=0; i<hsize; i++) {
     shift = lft ? hsize-i-1 : i;
     bitblt(b,Rect(r.origin.x+i,r.origin.y,r.origin.x+i+1,r.corner.y),
             b,Pt(r.origin.x+i,r.origin.y+muldiv(shift,dy,hsize)),
             F_STORE);
   }
*/
   for (i=0; i<hsize; i++) {
     shift = lft ? hsize-i-1 : i;
     shift = muldiv(shift,dy,hsize);
     for (j=(dir?vsize-1:0); (dir?j>=0:j<vsize); (dir?j--:j++)) {
       bit = bitmapbit(b,Pt(r.origin.x+i,r.origin.y+j));
       if (bit) point(b,Pt(r.origin.x+i,r.origin.y+j+shift),F_STORE);
       else point(b,Pt(r.origin.x+i,r.origin.y+j+shift),F_CLR);
     }
   }
}

OpRotPlus()
{
   int vsize,hsize,size;
   Rectangle r,rbuf;
   Bitmap *buffer;
   if (GetIconRect(&r)==0) return(0);
   hsize = horsize(r); vsize = versize(r); size = hsize+vsize;
   buffer = balloc(Rect(0,0,size,size));
   if (buffer == ((Bitmap *) 0)) return(0);
   rectf(buffer,buffer->rect,F_CLR);
   rbuf = rsubp(r,r.origin);
   bitblt(&display,raddp(r,icon.origin),buffer,rbuf.origin,F_XOR);
   horshear(buffer,rbuf,vsize,true);
   vershear(buffer,
            Rect(rbuf.origin.x,rbuf.origin.y,rbuf.corner.x+vsize,rbuf.corner.y),
            size,false);
   horshear(buffer,
            Rect(rbuf.origin.x,rbuf.corner.y-1,
                 rbuf.corner.x+vsize,rbuf.corner.y+hsize-1),
            -hsize,false);
   Erase(r);
   FetchIcon(buffer,
          Rect(rbuf.origin.x,rbuf.corner.y-1,
               rbuf.origin.x+vsize,rbuf.corner.y+hsize-1),
          add(r.origin,sub(Pt(hsize/2,vsize/2),Pt(vsize/2,hsize/2))),
          Rect(0,0,Xblocks,Yblocks));
   bfree(buffer);
}

OpRotMinus()
{
   int vsize,hsize,size;
   Rectangle r,rbuf;
   Bitmap *buffer;
   if (GetIconRect(&r)==0) return(0);
   hsize = horsize(r); vsize = versize(r); size = hsize+vsize;
   buffer = balloc(Rect(0,0,size,size));
   if (buffer == ((Bitmap *) 0)) return(0);
   rectf(buffer,buffer->rect,F_CLR);
   rbuf = raddp(r,sub(Pt(vsize,0),r.origin));
   bitblt(&display,raddp(r,icon.origin),buffer,rbuf.origin,F_XOR);
   horshear(buffer,rbuf,-vsize,true);
   vershear(buffer,
            Rect(rbuf.origin.x-vsize,rbuf.origin.y,rbuf.corner.x,rbuf.corner.y),
            size,true);
   horshear(buffer,
            Rect(rbuf.origin.x-vsize,rbuf.corner.y-1,
                 rbuf.corner.x,rbuf.corner.y+hsize-1),
            hsize,false);
   Erase(r);
   FetchIcon(buffer,
          Rect(rbuf.corner.x-vsize,rbuf.corner.y-1,
               rbuf.corner.x,rbuf.corner.y+hsize-1),
          add(r.origin,sub(Pt(hsize/2,vsize/2),Pt(vsize/2,hsize/2))),
          Rect(0,0,Xblocks,Yblocks));
   bfree(buffer);
}

OpReflY()
{
   Rectangle r;
   int i,j;
   bool bit1,bit2;
   if (GetIconRect(&r)==0) return(0);
   for (i=r.origin.x; i<r.corner.x; i+=1)
     for (j=0; j<(versize(r)/2); j+=1) {
        bit1 = geticonpoint(Pt(i,r.origin.y+j));
        bit2 = geticonpoint(Pt(i,r.corner.y-1-j));
        IconOp(bit1,Pt(i,r.corner.y-1-j),I_STORE);
        IconOp(bit2,Pt(i,r.origin.y+j),I_STORE);
     }
}

OpReflX()
{
   Rectangle r;
   int i,j;
   bool bit1,bit2;
   if (GetIconRect(&r)==0) return(0);
   for (i=0; i<(horsize(r)/2); i+=1)
     for (j=r.origin.y; j<r.corner.y; j+=1) {
        bit1 = geticonpoint(Pt(r.origin.x+i,j));
        bit2 = geticonpoint(Pt(r.corner.x-1-i,j));
        IconOp(bit1,Pt(r.corner.x-1-i,j),I_STORE);
        IconOp(bit2,Pt(r.origin.x+i,j),I_STORE);
     }
}

OpBlit(srcop,dstop)
int srcop,dstop;
{
   Rectangle r;
   Point p;
   if (GetIconRect(&r)==0) return(0);
   p = TrackBorder(r,true).origin;
   if (button23()) {
     while (button23());
     IconBitBlit(r,p,Rect(0,0,Xblocks,Yblocks),srcop,dstop);
   } else while (button1());
}


OpGeneralBlit()
{
           int srcop, dstop;
           cursswitch(&menucursor);
           while (!button123()) wait(MOUSE);
           cursswitch((Texture *) 0);
           if (!button3()) return(0);
           switch (menuhit(&blitsrcmenu,3)) {
             case NOHIT:
               srcop = I_NULL;
               break;
             case SRC_ID:
               srcop = I_OR;
               break;
             case SRC_CLR:
               srcop = I_CLR;
               break;
           }
           if (srcop==I_NULL) {cursswitch((Texture *) 0); return(0);}
           cursswitch(&menucursor);
           while (!button123()) wait(MOUSE);
           cursswitch((Texture *) 0);
           if (!button3()) return(0);
           switch (menuhit(&blitdstmenu,3)) {
             case NOHIT:
               dstop = I_NULL;
               break;
             case DST_STORE:
               dstop = I_STORE;
               break;
             case DST_OR:
               dstop = I_OR;
               break;
             case DST_XOR:
               dstop = I_XOR;
               break;
             case DST_CLR:
               dstop = I_CLR;
               break;
           }
           if (dstop==I_NULL) {cursswitch((Texture *) 0); return(0);}
           OpBlit(srcop,dstop);
}

Erase(r)
Rectangle r;
{
   int i,j;
   for (j = r.origin.y ; j < r.corner.y ; j++)
     for (i = r.origin.x ; i < r.corner.x ; i++)
       if (geticonpoint(Pt(i,j)) == true) flipiconpoint(Pt(i,j));
}

OpErase()
{
   Rectangle r;
   if (GetIconRect(&r)==0) return(0);
   Erase(r);
}

OpInvert()
{
   Rectangle r;
   int i,j;
   if (GetIconRect(&r)==0) return(0);
   for (j = r.origin.y ; j < r.corner.y ; j++)
     for (i = r.origin.x ; i < r.corner.x ; i++)
       flipiconpoint(Pt(i,j));
}

StoreIcon(bitmap,rect)
Bitmap *bitmap;
Rectangle rect;
{
   bitblt(&display,icon,bitmap,rect.origin,F_STORE);
}

FetchIcon(bitmap,r,p,clip)
Bitmap *bitmap;
Rectangle r;
Point p;
Rectangle clip;
{
   int i,j,hsize,vsize;
   bool bit;
   Point pij,srcp;
   if (!rectclip(&r,rsubp(clip,sub(p,r.origin)))) r = nullrect;
   hsize = horsize(r);
   vsize = versize(r);
   for(j=0; j<vsize; j++)
     for (i=0; i<hsize; i++) {
       bit = bitmapbit(bitmap,add(r.origin,Pt(i,j)));
       pij = add(p,Pt(i,j));
       if ((geticonpoint(pij) == false) && (bit == true))
         flipiconpoint(pij);
     }
}

GetFNAME()
{
   Point p;
   cursswitch(&deadmouse);
   p = string(&defont,"File: ",&display,add(icon.corner,Pt(15,-20)),F_XOR);
   getstr(FNAME,p);
   p = string(&defont,"File: ",&display,add(icon.corner,Pt(15,-20)),F_XOR);
   string(&defont,FNAME,&display,p,F_XOR);
   cursswitch((Texture *) 0);
}

Bitmap *pickupmap;

PickUpCursor()
{
   Texture *oldcursor;
   Rectangle r;
   r = TrackBorder(raddp(rect16x16,IconPoint(mouse.xy)),true);
   r = raddp(r,icon.origin);
   bitblt(&display,r,pickupmap,Pt(0,0),F_STORE);
   oldcursor = cursswitch(pickupmap->base);
   while (button123());
   while (!button123());
   while (button123());
   cursswitch(oldcursor);
}

char SFbuffer[100];
char *SFlist[] = {		/* Where to look */
    "",				/* Look in local directory first */
    "/usr/jerq/icon/16x16/",	/* Ick! "/usr/jerq/icon" should be a parameter */
    "/usr/jerq/icon/texture/",
    "/usr/jerq/icon/large/",
    (char *) 0
};

FILE *SearchFile(filename,mode,filefound)
char *filename, *mode, **filefound;
{
   FILE *fp;
   char **sf = SFlist;
   int namez = strlen(filename);
   *filefound = SFbuffer;
   fp = (FILE *) 0;
   for (; (fp == (FILE *) 0) && (*sf != (char *) 0); sf++) {
     if ((strlen(*sf) + namez) < sizeof(SFbuffer)) {
       strcpy(SFbuffer, *sf);
       strcat(SFbuffer, filename);
       fp = fopen(SFbuffer, mode);
     }
   }
   return(fp);
}

Rectangle OpLoad(bitmap,filename)
Bitmap *bitmap;
char *filename;
{
   FILE *fp;
   Rectangle rect;
   Texture *oldcursor;
   char *filefound;
   int ch,i,j;
   int xsize,ysize;
   rect = bitmap->rect;
   oldcursor = cursswitch(&clock);
   fp = SearchFile(filename,"r",&filefound);
   if (fp == ((FILE *) 0)) {cursswitch((Texture *) 0); return(nullrect);}
   ch = getc(fp);
   if (((ch>='0')&&(ch<='9')) || ((ch>='A')&&(ch<='F')) || ((ch>='a')&&(ch<='f'))) {
     if (((ch=getc(fp))!='x') && (ch!='X')) {
       /* old format */
       fclose(fp); fp = fopen(filefound,"r");
       if (fp == ((FILE *) 0)) {cursswitch((Texture *) 0); return(nullrect);}
       i = rect.origin.x; j = rect.origin.y;
       xsize = 0;
       while ((ch=getc(fp))!=EOF) {
         if (((ch>='0')&&(ch<='9')) || ((ch>='A')&&(ch<='F')) || ((ch>='a')&&(ch<='f'))) {
           putnibble(ch,bitmap,rect,Pt(i,j));
           i = i+4;
         } else if (ch=='\n') {
           xsize = max(xsize,i-rect.origin.x);
           i = rect.origin.x;
           j++;
         } else break;
       }
       ysize = j-rect.origin.y;
     } else {
       i = rect.origin.x; j = rect.origin.y;
       xsize = 0;
       for (;;) {
         putnibble(getc(fp),bitmap,rect,Pt(i,j)); i+=4;
         putnibble(getc(fp),bitmap,rect,Pt(i,j)); i+=4;
         putnibble(getc(fp),bitmap,rect,Pt(i,j)); i+=4;
         putnibble(getc(fp),bitmap,rect,Pt(i,j)); i+=4;
         getc(fp); /* ',' */
         ch = getc(fp);
         if (ch=='0') {
           getc(fp); /* 'x' */
         } else if (ch=='\n') {
           xsize = max(xsize,i-rect.origin.x);
           i = rect.origin.x;
           j++;
           if (getc(fp) == EOF) break;	/* '0' (if not EOF) */
           getc(fp);			/* 'x' */
         } else break;
       }
       ysize = j-rect.origin.y;
     }
   } else {
     while ((ch!='{')&&(ch!=EOF)) ch=getc(fp);
     for (j=rect.origin.y; j<rect.origin.y+16; j++) {
       while (((ch=getc(fp))!='x')&&(ch!='X')&&(ch!=EOF)) {};
       putnibble(getc(fp),bitmap,rect,Pt(rect.origin.x,j));
       putnibble(getc(fp),bitmap,rect,Pt(rect.origin.x+4,j));
       putnibble(getc(fp),bitmap,rect,Pt(rect.origin.x+8,j));
       putnibble(getc(fp),bitmap,rect,Pt(rect.origin.x+12,j));
     }
     xsize = ysize = 16;
   }
   fclose(fp);
   rect.origin.x = 0; rect.origin.y = 0; 
   rect.corner.x = xsize; rect.corner.y = ysize;
   cursswitch(oldcursor);
   return(rect);
}

OpRead(bitmap)
Bitmap *bitmap;
{
   Rectangle rect;
   Bitmap *buffer;
   Point p;
   GetFNAME();
   if (!FNAME[0]) return(0);
   buffer = balloc(bitmap->rect);
   if (buffer == ((Bitmap *) 0)) {cursswitch((Texture *) 0); return(0);}
   rectf(buffer,buffer->rect,F_CLR);
   rect = OpLoad(buffer,FNAME);
   if (!eqrect(rect,nullrect)) {
     p = TrackBorder(raddp(rect,IconPoint(mouse.xy)),true).origin;
     while (button3());
     bitblt(buffer,buffer->rect,bitmap,p,F_OR);
     bfree(buffer);
   }
}

#define BUFSIZE 100
char buffer[BUFSIZE];
char *bufend;

bclear()
{
	bufend = buffer;
}

bsend(fp)
FILE *fp;
{
	*bufend = '\0';
	fputs(buffer,fp);
	bclear();
}

bputc(c,fp)
char c;
FILE *fp;
{
   if (bufend >= buffer+BUFSIZE-3) bsend(fp);
   *bufend++ = c;
}

OpWrite(bitmap,drawrect)
Bitmap *bitmap;
Rectangle drawrect;
{
   FILE *fp;
   Rectangle r;
   Point p;
   int i,j,butt;
   butt = GetIconRect(&r);
   if (butt==3) { /* write in hex format */
     GetFNAME();
     if (!FNAME[0]) return(0);
     fp = fopen(FNAME,"w");
     if (fp == ((FILE *) 0)) return(0);
     rectf(&display,drawrect,F_XOR);
     bclear();
     for (j = r.origin.y; j<r.corner.y; j++) {
       for (i=r.origin.x; i<r.corner.x; i+=16) {
         bputc('0',fp);bputc('x',fp);
         bputc(getnibble(bitmap,Pt(i,j)),fp);
	 bputc(getnibble(bitmap,Pt(i+4,j)),fp);
	 bputc(getnibble(bitmap,Pt(i+8,j)),fp);
	 bputc(getnibble(bitmap,Pt(i+12,j)),fp);
         bputc(',',fp);
       }
       bputc('\n',fp);
       bsend(fp);
     }
     fclose(fp);
     rectf(&display,drawrect,F_XOR);
   } else if (butt==2) { /* write in texture format */
     GetFNAME();
     if (!FNAME[0]) return(0);
     fp = fopen(FNAME,"w");
     if (fp == ((FILE *) 0)) return(0);
     rectf(&display,drawrect,F_XOR);
     fputs("Texture ",fp); fputs(FNAME,fp); fputs(" = {\n",fp);
     j = r.origin.y; i = r.origin.x;
     bclear();
     while (j < r.corner.y) {
       if (((j-r.origin.y)%4) == 0) bputc('\t',fp);
       bputc(' ',fp);bputc('0',fp);bputc('x',fp);
       bputc(getnibble(bitmap,Pt(i,j)),fp);
       bputc(getnibble(bitmap,Pt(i+4,j)),fp);
       bputc(getnibble(bitmap,Pt(i+8,j)),fp);
       bputc(getnibble(bitmap,Pt(i+12,j)),fp);
       bputc(',',fp);
       if (((j-r.origin.y)%4) == 3) bputc('\n',fp);
       j = j+1;
       bsend(fp);
     }
     fputs("};\n",fp);
     fclose(fp);
     rectf(&display,drawrect,F_XOR);
   }
}

OpTexture()
{
   Bitmap *buffer;
   Rectangle source,dest;
   Point target;
   int repx,repy,i,j,hsize,vsize;
   if (GetIconRect(&source)==0) return(0);
   if (GetIconRect(&dest)==0) return(0);
   hsize = horsize(source);
   vsize = versize(source);
   if ((hsize==0) || (vsize==0)) return(0);
   buffer = balloc(source);
   if (buffer == ((Bitmap *) 0)) return(0);
   bitblt(&display,raddp(source,icon.origin),buffer,buffer->rect.origin,F_STORE);
   repx = horsize(dest)/hsize;
   repy = versize(dest)/vsize;
   for (j=0; j<=repy; j++) 
     for (i=0; i<=repx; i++)
       FetchIcon(buffer,buffer->rect,add(dest.origin,Pt(i*hsize,j*vsize)),dest);
   bfree(buffer);
}

HorShear(r,dx,top)
Rectangle r;
int dx;
bool top;
{
   int j,vsize,shift;
   vsize = versize(r);
   for (j=0; j<vsize; j++) {
     shift = top ? vsize-j-1 : j;
     IconBitBlit(Rect(r.origin.x,r.origin.y+j,r.corner.x,r.origin.y+j+1),
                 Pt(r.origin.x+muldiv(shift,dx,vsize),r.origin.y+j),
                 Rect(0,0,Xblocks,Yblocks),
                 I_CLR,I_OR);
   }
}

VerShear(r,dy,lft)
Rectangle r;
int dy;
bool lft;
{
   int i,hsize,shift;
   hsize = horsize(r);
   for (i=0; i<hsize; i++) {
     shift = lft ? hsize-i-1 : i;
     IconBitBlit(Rect(r.origin.x+i,r.origin.y,r.origin.x+i+1,r.corner.y),
                 Pt(r.origin.x+i,r.origin.y+muldiv(shift,dy,hsize)),
                 Rect(0,0,Xblocks,Yblocks),
                 I_CLR,I_OR);
   }
}

OpHorShear()
{
   Rectangle r;
   int dx;
   bool top;
   Point p,nearcorner;
   if (GetIconRect(&r)==0) return(0);
   if ((horsize(r)==0) || (versize(r)==0)) return(0);
   if (!GetIconPoint(&p)) return(0);
   nearcorner = nearestcorner(r,p);
   dx = p.x - nearcorner.x;
   top = (nearcorner.y == r.origin.y);
   HorShear(r,dx,top);
}

OpVerShear()
{
   Rectangle r;
   int dy;
   bool lft;
   Point p,nearcorner;
   if (GetIconRect(&r)==0) return(0);
   if ((horsize(r)==0) || (versize(r)==0)) return(0);
   if (!GetIconPoint(&p)) return(0);
   nearcorner = nearestcorner(r,p);
   dy = p.y - nearcorner.y;
   lft = (nearcorner.x == r.origin.x);
   VerShear(r,dy,lft);
}

Stretch(sb,sr,db,dr,op)
Bitmap *sb,*db;
Rectangle sr,dr;
Code op;
{
   int i,j,shsize,svsize,dhsize,dvsize;
   shsize = horsize(sr);
   svsize = versize(sr);
   dhsize = horsize(dr);
   dvsize = versize(dr);
   for (j=0; j<svsize; j++)
     for (i=0; i<shsize; i++)
       bitblt(sb,
              Rect(sr.origin.x+i,sr.origin.y+j,sr.origin.x+i+1,sr.origin.y+j+1),
              db,
              Pt(dr.origin.x+muldiv(dhsize,i,shsize),
                 dr.origin.y+muldiv(dvsize,j,svsize)),
              op);
}

OpStretch()
{
   Bitmap *buffer;
   Rectangle source,dest;
   if (GetIconRect(&source)==0) return(0);
   if ((horsize(source)==0) || (versize(source)==0)) return(0);
   if (GetIconRect(&dest)==0) return(0);
   if ((horsize(dest)==0) || (versize(dest)==0)) return(0);
   buffer = balloc(dest);
   if (buffer == ((Bitmap *) 0)) return(0);
   rectf(buffer,buffer->rect,F_CLR);
   Stretch(&display,raddp(source,icon.origin),buffer,buffer->rect,F_XOR);
   IconBitBlit(source,source.origin,Rect(0,0,0,0),F_CLR,F_CLR);
   FetchIcon(buffer,buffer->rect,dest.origin,dest);
   bfree(buffer);
}

DrawGrid()
{
   Point p;
   register int i,j;
   for (j=0; j<=Yblocks; j+=16) {
     p=add(ICON.origin,Pt(0,j*Ysize+1));
     segment(&display,p,Pt(ICON.corner.x,p.y),F_XOR);
   }
   for (j=8; j<=Yblocks; j+=16) {
     p=add(ICON.origin,Pt(0,j*Ysize+1));
     for (i=0; i<=Xblocks*Xsize; i+=2)
       point(&display,Pt(p.x+i,p.y),F_XOR);
   }
   for (i=0; i<=Xblocks; i+=16) {
     p=add(ICON.origin,Pt(i*Xsize+1,0));
     segment(&display,p,Pt(p.x,ICON.corner.y),F_XOR);
   }
   for (i=8; i<=Xblocks; i+=16) {
     p=add(ICON.origin,Pt(i*Xsize+1,0));
     for (j=0; j<=Yblocks*Ysize; j+=2)
       point(&display,Pt(p.x,p.y+j),F_XOR);
   }
}

helpline(b,i,icon,str)
Bitmap *b;
int i;
Texture *icon;
char *str;
{
   texture(b,raddp(rect16x16,Pt(0,16*i)),icon,F_XOR);
   string(&defont,str,b,Pt(32,16*i),F_XOR);
}

HelpSorry()
{
     string(&defont,"Not enough space on blit",
            &display,add(icon.corner,Pt(15,-20)),F_XOR);
}

Help()
{
   Bitmap *buffer;
   buffer = balloc(Rect(-2,-2,280,464));
   if (buffer == 0) {
     HelpSorry();
     while (!button123()) wait(MOUSE);
     HelpSorry();
     while (button123()); 
     return(0);
   }
   rectf(buffer,buffer->rect,F_CLR);
   outline(buffer,Rpt(buffer->rect.origin,sub(buffer->rect.corner,Pt(1,1))));
   helpline(buffer,0,&white,"button 1: draw");
   helpline(buffer,1,&white,"button 2: undraw");
   helpline(buffer,2,&white,"");
   helpline(buffer,3,&Cmove,"move");
   helpline(buffer,4,&Ccopy,"copy");
   helpline(buffer,5,&Cinvert,"invert");
   helpline(buffer,6,&Cerase,"erase");
   helpline(buffer,7,&Creflx,"reflect x");
   helpline(buffer,8,&Crefly,"reflect y");
   helpline(buffer,9,&Crotplus,"rotate +");
   helpline(buffer,10,&Crotminus,"rotate -");
   helpline(buffer,11,&Cshearx,"shear x");
   helpline(buffer,12,&Csheary,"shear y");
   helpline(buffer,13,&Cstretch,"stretch");
   helpline(buffer,14,&Ctexture,"texture");
   helpline(buffer,15,&Cread,"read file");
   helpline(buffer,16,&Cgrid,"background grid");
   helpline(buffer,17,&Ccursor,"pick cursor icon");
   helpline(buffer,18,&Cwrite,"write file");
   helpline(buffer,19,&Cblit,"bitblit");
   helpline(buffer,20,&Chelp,"press a button to continue");
   helpline(buffer,21,&Cexit,"exit (confirm butt 3)");
   helpline(buffer,22,&white,"");
   helpline(buffer,23,&clock,"wait");
   helpline(buffer,24,&deadmouse,"mouse inactive");
   helpline(buffer,25,&menucursor,  "menu on button 3");
   helpline(buffer,26,&sweepcursor,"sweep rect (butt 3)");
   helpline(buffer,27,&sweeportrack,"sweep rect (butt 3) or");
   helpline(buffer,28,&white,"get 16x16 frame (butt 2)");
   screenswap(buffer,buffer->rect,raddp(buffer->rect,add(Drect.origin,Pt(5,5))));
   while (!button123());
   screenswap(buffer,buffer->rect,raddp(buffer->rect,add(Drect.origin,Pt(5,5))));
   while (button123());
   bfree(buffer);
}

Icon(sourcebitmap,sourcerect,drawrect)
Bitmap *sourcebitmap;
Rectangle sourcerect, drawrect;
{
   Bitmap *b;
   Point p,cur,hit;
   int i,j;

   if (!rectclip(&sourcerect,sourcebitmap->rect)) return(0);

   Xblocks = horsize(sourcerect);
   Yblocks = versize(sourcerect);

   Xsize = (horsize(drawrect)-(Xblocks+9))/Xblocks;
   Ysize = (versize(drawrect)-(Yblocks+9))/Yblocks;

   if ((Xsize==0) || (Ysize==0)) return(0);

   Ysize = (Xsize = (Xsize<Ysize)?Xsize:Ysize);

   icon.origin = add(drawrect.origin,Pt(5,5));
   icon.corner = add(icon.origin,Pt(Xblocks,Yblocks));

   ICON.origin = sub(drawrect.corner,Pt(3+Xsize*Xblocks,3+Ysize*Yblocks));
   ICON.corner = add(ICON.origin,Pt(Xsize*Xblocks,Ysize*Yblocks));

   modx = ICON.origin.x % Xsize;
   divx = ICON.origin.x / Xsize;
   mody = ICON.origin.y % Ysize;
   divy = ICON.origin.y / Ysize;

   stipple(drawrect);

   for (j=0; j<=Yblocks; j++) {
     p=add(ICON.origin,Pt(0,j*Ysize));
     segment(&display,p,Pt(ICON.corner.x,p.y),F_OR);
   }

   for (i=0; i<=Xblocks; i++) {
     p=add(ICON.origin,Pt(i*Xsize,0));
     segment(&display,p,Pt(p.x,ICON.corner.y),F_OR);
   }

   FetchIcon(sourcebitmap,sourcerect,Pt(0,0),Rect(0,0,Xblocks,Yblocks));

   cur.x = 0; cur.y = 0;

   for (;;) {
     wait(MOUSE);
     if (P->state&RESHAPED) return(0);
     if (button1()) {
       while(button1()) {
         if (ptinrect((p=mouse.xy),ICON)) {
           p = sub(p,ICON.origin);
           cur.x = p.x/Xsize;
           cur.y = p.y/Ysize;
           if (geticonpoint(cur) == false) flipiconpoint(cur);
         }
       }
     } else if (button2()) {
       while(button2()) {
         if (ptinrect((p=mouse.xy),ICON)) {
           p = sub(p,ICON.origin);
           cur.x = p.x/Xsize;
           cur.y = p.y/Ysize;
           if (geticonpoint(cur) == true) flipiconpoint(cur);
         }
       }
     }
     if (button3()) {
       hit = imenuhit(imenu);
       switch (5*hit.y+hit.x) {
         case BLIT:
           OpGeneralBlit();
           break;
         case MOVE:
           OpBlit(I_CLR,I_OR);
           break;
         case COPY:
           OpBlit(I_OR,I_OR);
           break;
         case ERASE:
           OpErase();
           break;
         case INVERT:
           OpInvert();
           break;
         case REFLECTX:
           OpReflX();
           break;
         case REFLECTY:
           OpReflY();
           break;
         case ROTATEPLUS:
           OpRotPlus();
           break;
         case ROTATEMINUS:
           OpRotMinus();
           break;
         case SHEARX:
           OpHorShear();
           break;
         case SHEARY:
           OpVerShear();
           break;
         case STRETCH:
           OpStretch();
           break;
         case TEXTURE:
           OpTexture();
           break;
         case GRID:
           DrawGrid();
           break;
         case PICK:
           PickUpCursor();
           break;
         case READ:
           b = balloc(sourcebitmap->rect);
           if (b == ((Bitmap *) 0)) break;
           StoreIcon(b,b->rect);
           OpRead(b);
           FetchIcon(b,b->rect,Pt(0,0),Rect(0,0,Xblocks,Yblocks));
           bfree(b);
           break;
         case WRITE:
           b = balloc(sourcebitmap->rect);
           if (b == ((Bitmap *) 0)) break;
           StoreIcon(b,b->rect);
           OpWrite(b,drawrect);
           bfree(b);
           break;
         case SPARE1:
           break;
         case HELP:
           Help();
           break;
         case EXIT:
           cursswitch(&Cexit);
           while (!button123()) {wait(CPU); wait(MOUSE);}
           if (button3()) {
             while(button3()) {wait(CPU); wait(MOUSE);}
             cursswitch((Texture *) 0);
             StoreIcon(sourcebitmap,sourcerect);
             return(0);
           } else {
             while(button12()) {wait(CPU); wait(MOUSE);}
             cursswitch((Texture *) 0);
             break;
           }
       }
     }
   }
}


getstr(s,p)
char *s;
Point p;
{
   char c,*t;
   static char str[]="x";
   t = s;
   for (;;) {
     wait(KBD);
     if (((c=kbdchar()) == '\r') || (c == '\n')) {
       *s = '\0';
       return;
     }
     if (c == '\b') {
       if (s>t) {
         str[0] = *(--s);
         string(&defont,str,&display,(p = sub(p,Pt(9,0))),F_XOR);
       }
     } else if ((c >= '!') && (c <= '~')) {
       if (s-t<50) {
         *s++ = (str[0] = c);
         p = string(&defont,str,&display,p,F_XOR);
       }
     }
   }
}

main(argc,argv)
char *argv[];
{

  Bitmap *p;
  int xsize=XSIZE;
  int ysize=YSIZE;
  char *InitFile=NULL;

  nullpoint.x = 0;
  nullpoint.y = 0;
  point16x16.x = 16;
  point16x16.y = 16;
  nullrect.origin = nullpoint;
  nullrect.corner = nullpoint;
  rect16x16.origin = nullpoint;
  rect16x16.corner = point16x16;

  bittester = balloc(Rect(0,0,1,1));
  pickupmap = balloc(Rect(0,0,16,16));

  request(MOUSE|SEND|KBD|RCV);

  while (--argc) {
    ++argv;
    if (**argv=='-') {
       switch(argv[0][1]) {
         case 'x':
           argc--;
	   xsize = atoi(*++argv);
           break;
         case 'y':
           argc--;
	   ysize = atoi(*++argv);
           break;
       }
    } else {
      InitFile = argv[0];
    }
  }

  /* *****  while (true) imenuhit(imenu); */ 

  if (xsize <= 0) xsize = XSIZE;
  if (ysize <= 0) ysize = YSIZE;

  p = balloc(Rect(0,0,xsize,ysize));
  if (p == ((Bitmap *) 0)) exit();
  rectf(p,p->rect,F_CLR);

  if (InitFile) {
    OpLoad(p,InitFile);
  }

  do {
    P->state&=~RESHAPED;
    Icon(p,p->rect,Drect);
  } while (P->state&RESHAPED);
  bfree(p);

  exit();

}
