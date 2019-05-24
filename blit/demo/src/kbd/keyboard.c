
#include <jerq.h>
#include <jerqio.h>
#include <font.h>
#include "keyboard.h"

typedef int bool;
#define TRUE  1
#define FALSE 0

#define NUL '\000'
#define SOH '\001'
#define STX '\002'
#define ETX '\003'
#define EOT '\004'
#define ENQ '\005'
#define ACK '\006'
#define BEL '\007'
#define BS  '\010'
#define HT  '\011'
#define NL  '\012'
#define VT  '\013'
#define NP  '\014'
#define CR  '\015'
#define SO  '\016'
#define SI  '\017'
#define DLE '\020'
#define DC1 '\021'
#define DC2 '\022'
#define DC3 '\023'
#define DC4 '\024'
#define NAK '\025'
#define SYN '\026'
#define ETB '\027'
#define CAN '\030'
#define EM  '\031'
#define SUB '\032'
#define ESC '\033'
#define FS  '\034'
#define GS  '\035'
#define RS  '\036'
#define US  '\037'
#define DEL '\177'

#define KEYMARGIN   3

struct Keyboard *KbdPool, *CurKbd;
int fontcounter;

char lowerchars[4][15] =
  { {ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '`', BS },
    {HT , 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', CR , DEL},
    {' ', ' ', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','\\',' '},
    {' ', ' ', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', ' ', NL , ' '} };

char upperchars[4][15] =
  { {ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '~', BS },
    {HT , 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', CR , DEL},
    {' ', ' ', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '|', ' '},
    {' ', ' ', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', ' ', NL , ' '} };

char lowercontrolchars[4][15] =
  { {ESC, '1', NUL, ' ', '4', '5', RS , '7', '8', '9', '0', US , '=', '`', BS },
    {HT , DC1, ETB, ENQ, DC2, DC4, EM , NAK, HT , SI , DLE, ESC, GS , CR , DEL},
    {' ', ' ', SOH, DC3, EOT, ACK, BEL, BS , NL , VT , NP , ';', '\'',FS , ' '},
    {' ', ' ', SUB, CAN, ETX, SYN, STX, SO , CR , ',', '.', '/', ' ', NL , ' '} };

char uppercontrolchars[4][15] =
  { {ESC, '!', NUL, ' ', '$', '%', RS , '&', '*', '(', ')', US , '+', '~', BS },
    {HT , DC1, ETB, ENQ, DC2, DC4, EM , NAK, HT , SI , DLE, ESC, GS , CR , DEL},
    {' ', ' ', SOH, DC3, EOT, ACK, BEL, BS , NL , VT , NP , ':', '"', FS , ' '},
    {' ', ' ', SUB, CAN, ETX, SYN, STX, SO , CR , '<', '>', '?', ' ', NL , ' '} };

char keynoofchar[128][2] =
  {{ -3,-1},{  3,-3},{  7,-4},{  5,-4},{  5,-3},{  4,-2},{  6,-3},{  7,-3},
   { 15, 1},{  1, 2},{ 14, 4},{ 10,-3},{ 11,-3},{ 14, 2},{  8,-4},{ 10,-2},
   { 11,-2},{  2,-2},{  5,-2},{  4,-3},{  6,-2},{  8,-2},{  6,-4},{  3,-2},
   {  4,-4},{  7,-2},{  3,-4},{  1, 1},{ 14,-3},{ 13,-2},{ -7,-1},{-12,-1},
   {  1, 5},{ -2, 1},{-13, 3},{ -4, 1},{ -5, 1},{ -6, 1},{ -8, 1},{ 13, 3},
   {-10, 1},{-11, 1},{ -9, 1},{-13, 1},{ 10, 4},{ 12, 1},{ 11, 4},{ 12, 4},
   { 11, 1},{  2, 1},{  3, 1},{  4, 1},{  5, 1},{  6, 1},{  7, 1},{  8, 1},
   {  9, 1},{ 10, 1},{-12, 3},{ 12, 3},{-10, 4},{ 13, 1},{-11, 4},{-12, 4},
   { -3, 1},{ -3, 3},{ -7, 4},{ -5, 4},{ -5, 3},{ -4, 2},{ -6, 3},{ -7, 3},
   { -8, 3},{ -9, 2},{ -9, 3},{-10, 3},{-11, 3},{ -9, 4},{ -8, 4},{-10, 2},
   {-11, 2},{ -2, 2},{ -5, 2},{ -4, 3},{ -6, 2},{ -8, 2},{ -6, 4},{ -3, 2},
   { -4, 4},{ -7, 2},{ -3, 4},{ 12, 2},{ 14, 3},{ 13, 2},{ -7, 1},{-12, 1},
   { 14, 1},{  3, 3},{  7, 4},{  5, 4},{  5, 3},{  4, 2},{  6, 3},{  7, 3},
   {  8, 3},{  9, 2},{  9, 3},{ 10, 3},{ 11, 3},{  9, 4},{  8, 4},{ 10, 2},
   { 11, 2},{  2, 2},{  5, 2},{  4, 3},{  6, 2},{  8, 2},{  6, 4},{  3, 2},
   {  4, 4},{  7, 2},{  3, 4},{-12, 2},{-14, 3},{-13, 2},{-14, 1},{ 15, 2}
  };

KbdDrawChar (c, fp, db, p)
/* draw a char with reference point at p */
char c;
Font *fp;
Bitmap *db;
Point p;
{
   Rectangle r;
   if (c<=fp->n) {
     Fontchar *i=fp->info+c;
     r.origin.y = i->top;
     r.corner.y = i->bottom;
     r.origin.x = i->x;
     r.corner.x = (i+1)->x;
     bitblt(fp->bits, r, db, Pt(p.x+i->left, p.y+r.origin.y-fp->ascent), F_XOR);
   }
}

DrawTMChar (c, fp, db, p)
/* draw a char with top-middle of its rectangle at p
   (its rectangle is as high as the font) */
char c;
Font * fp;
Bitmap *db;
Point p;
{
   Rectangle r;
   if (c<=fp->n) {
     Fontchar *i=fp->info+c;
     r.origin.y = i->top;
     r.corner.y = i->bottom;
     r.origin.x = i->x;
     r.corner.x = (i+1)->x;
     bitblt(fp->bits, r, db,
            Pt(p.x-((r.corner.x-r.origin.x)/2), p.y+r.origin.y),
            F_XOR);
   }
}

struct Keyboard *MakeKeyboard(fp,origin,pool)
Font *fp;
Point origin;
struct Keyboard *pool;
{
   int keysize;
   struct Keyboard *keyb;
   keyb = (struct Keyboard *) alloc(sizeof(struct Keyboard));
   keysize = fp->height+2*KEYMARGIN;
   keyb->origin = origin;
   keyb->enclosure.origin.x = keysize/4-keysize-5;
   keyb->enclosure.origin.y = -5;
   keyb->enclosure.corner.x = 16*keysize+5;
   keyb->enclosure.corner.y = 5*keysize+5;
   keyb->key.origin.x = 0;
   keyb->key.origin.y = 0;
   keyb->key.corner.x = keysize;
   keyb->key.corner.y = keysize;
   keyb->keysize = keysize;
   keyb->state = LOWERCASE_STATE;
   keyb->tempshift = FALSE;
   keyb->tempctrl = FALSE;
   keyb->fontcode = fontcounter; fontcounter += 1;
   keyb->font = fp;
   keyb->next = pool;
}

KbdChangeState(newstate,Kbd)
int newstate;
struct Keyboard *Kbd;
{
   if (!(newstate==Kbd->state)) {
     if (LocalTop(Kbd)) {
       DrawChars(Kbd);
       Kbd->state = newstate;
       DrawChars(Kbd);
     } else {
       Kbd->state = newstate;
       TopKbd(Kbd);
       DrawKeyboard(Kbd);
     }
   }
}

int UpperKey(ch)
int ch;
{
   return(
     (ch==ESC) || (ch==HT) || (ch==BS) || (ch==DEL) ||
     (ch==CR) || (ch==NL) || (ch==' ') || ((ch>='!') && (ch<='&')) ||
     ((ch>='(') && (ch<='+')) || (ch==':') || (ch=='<') ||
     ((ch>='>') && (ch<='Z')) || (ch=='^') || (ch=='_') ||
     ((ch>='{') && (ch<='~'))
   );
}

int LowerKey(ch)
int ch;
{
   return(
     (ch==ESC) || (ch==HT) || (ch==BS) || (ch==DEL) ||
     (ch==CR) || (ch==NL) || (ch==' ') || (ch=='\'') ||
     ((ch>=',') && (ch<='9')) || (ch==';') || (ch=='=') ||
     ((ch>='[') && (ch<=']')) || ((ch>='`') && (ch<='z'))
   );
}

int CtrlKey(ch)
int ch;
{
   return(
     (ch<=' ') || (ch=='1') || (ch=='4') || (ch=='5') ||
     ((ch>='7') && (ch<='9')) || (ch=='=') || (ch=='\'') || (ch=='`') ||
     (ch==';') ||  (ch==',') || ((ch>='.') && (ch<='0'))
   );
}

int CtrlShKey(ch)
int ch;
{
   return(
     (ch<=' ') || (ch=='!') || (ch=='$') || (ch=='%') || (ch=='&') ||
     ((ch>='(') && (ch<='+')) || (ch=='~') ||
     (ch==':') || (ch=='"') || (ch=='<') || (ch=='>') ||(ch=='?')
   );
}

outline(r)
Rectangle  r;
{
   if (r.origin.x == r.corner.x)
     if (r.origin.y == r.corner.y) {}
     else segment(&display,r.origin,Pt(r.origin.x,r.corner.y-1),F_XOR);
   else
     if (r.origin.y == r.corner.y)
     segment(&display,Pt(r.corner.x-1,r.origin.y),r.origin,F_XOR);
     else {
       segment(&display,r.origin,Pt(r.origin.x,r.corner.y-1),F_XOR);
       segment(&display,Pt(r.origin.x,r.corner.y-1),sub(r.corner,Pt(1,1)),F_XOR);
       segment(&display,sub(r.corner,Pt(1,1)),Pt(r.corner.x-1,r.origin.y),F_XOR);
       segment(&display,Pt(r.corner.x-1,r.origin.y),r.origin,F_XOR);
     }
}

border(r)
Rectangle r;
{
  outline(r);
  outline(inset(r,2));
}

KeyNoOfChar(ch,i,j,shift,control)
int ch, *i, *j, *shift, *control;
{
   *shift = 0;
   *control = 0;
   *i = keynoofchar[ch][0];
   if (*i<0) { *i = -(*i); *shift = 1;}
   *j = keynoofchar[ch][1];
   if (*j<0) { *j = -(*j); *control = 1;}
}

Rectangle KeyOfKeyNo(i,j,keyb)
int i,j;
struct Keyboard *keyb;
/*  Fills key with the rectangle i,j (according to the following table);
    key is in keyboard coordinates (origin at the top left of key 1,1).

     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     |1,1|2,1|3,1|4,1|5,1|6,1|7,1|8,1|9,1|0,1|1,1|2,1|3,1|4,1|5,1|6,1|
     +-----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-+
     | 1,2 |2,2|3,2|4,2|5,2|6,2|7,2|8,2|9,2|0,2|1,2|2,2|3,3|4,2|5,2|
  +---+-----+---+---+---+---+---+---+---+---+---+---+---+--+   +---+
  |1,3| 2,3 |3,3|4,3|5,3|6,3|7,3|8,3|9,3|0,3|1,3|2,3|3,3|      |4,3|
  +---+-------+---+---+---+---+---+---+---+---+---+---+-------+---++
  |1,4|  2,4  |3,4|4,4|5,4|6,4|7,4|8,4|9,4|0,4|1,4|2,4| 13,4  |4,4|
  +------------+-----------------------------------+--------------+
               |               1,5                 |
               +-----------------------------------+
*/
{
   int keysize;
   Rectangle key;
   keysize = keyb->keysize;

   if (j==1) { /* esc .. break */
     key.origin.x = (i-1)*keysize;
     key.origin.y = 0;
     key.corner.x = i*keysize;
     key.corner.y = keysize;
   } else if (j==2) {
     key.origin.y = keysize;
     key.corner.y = 2*keysize;
     if (i==1) {  /* tab */
       key.origin.x = 0;
       key.corner.x = 3*keysize/2;
     } else if (i==14) {  /* return */
       key.corner.y = 3*keysize;
       key.origin.x = 27*keysize/2;
       key.corner.x = 29*keysize/2;
     } else { /* Q .. ], del */
       key.origin.x = (2*(i-2)+3)*keysize/2;
       key.corner.x = (2*(i-1)+3)*keysize/2;
     }
   } else if (j==3) {
     key.origin.y = 2*keysize;
     key.corner.y = 3*keysize;
     if (i==1) { /* control */
        key.origin.x = -(3*keysize/4);
        key.corner.x = keysize/4;
     } else if (i==2) {  /* caps lock */
        key.origin.x = keysize/4;
        key.corner.x = 7*keysize/4;
     } else if (i==14) { /* \ */
        key.origin.x = 29*keysize/2;
        key.corner.x = 31*keysize/2;
     } else { /* A .. ' */
        key.origin.x = (4*(i-3)+7)*keysize/4;
        key.corner.x = (4*(i-2)+7)*keysize/4;
     }
   } else if (j==4) {
     key.origin.y = 3*keysize;
     key.corner.y = 4*keysize;
     if (i==1) {  /* no scrl */
       key.origin.x = -(3*keysize/4);
       key.corner.x = keysize/4;
     } else if (i==2) { /* left shift */
       key.origin.x = keysize/4;
       key.corner.x = 9*keysize/4;
     } else if (i==13) {  /* right shift */
       key.origin.x = 49*keysize/4;
       key.corner.x = 57*keysize/4;
     } else if (i==14) {  /* line feed */
       key.origin.x = 57*keysize/4;
       key.corner.x = 61*keysize/4;
     } else { /* Z .. / */
       key.origin.x = (4*(i-3)+9)*keysize/4;
       key.corner.x = (4*(i-2)+9)*keysize/4;
     }
   } else { /* space bar */
       key.origin.y = 4*keysize;
       key.corner.y = 5*keysize;
       key.origin.x = 5*keysize/2;
       key.corner.x = 23*keysize/2;
   }
   return(key);
}

KeyNoOfPos (p,keyb,i,j)
/* The key number of p (in keyboard coordinates) in keyb is assigned to i,j */
Point p;
struct Keyboard *keyb;
int *i,*j;
{
   int keysize;
   keysize = keyb->keysize;
   *i = 0; *j = 0;
   if (p.y<0) {
   } else if (p.y<keysize) {
     if ((p.x>=0) && (p.x<16*keysize)) {
       *j = 1;
       *i = p.x/keysize + 1;
     }
   } else if (p.y<2*keysize) {
     if ((p.x>=0) && (p.x<31*keysize/2)) {
       *j = 2;
       if (p.x<3*keysize/2) *i = 1;
       else *i = (p.x-keysize/2)/keysize + 1;
     }
   } else if (p.y<3*keysize) {
     if ((p.x>=-(3*keysize/4)) && (p.x<31*keysize/2)) {
       *j = 3;
       if (p.x<keysize/4) *i = 1;
       else if (p.x<7*keysize/4) *i = 2;
       else if (p.x>=29*keysize/2) *i = 14;
       else if (p.x>=27*keysize/2) { *j = 2; *i = 14; }
       else if (p.x>=51*keysize/4) *j = 0;
       else *i = (p.x-3*keysize/4)/keysize + 2;
     }
   } else if (p.y<4*keysize) {
     if ((p.x>=-(3*keysize/4)) && (p.x<61*keysize/4)) {
       *j = 4;
       if (p.x<keysize/4) *i = 1;
       else if (p.x<9*keysize/4) *i = 2;
       else if (p.x>=57*keysize/4) *i = 14;
       else if (p.x>=49*keysize/4) *i = 13;
       else *i = (p.x-5*keysize/4)/keysize + 2;
     }
   } else if (p.y<5*keysize) {
     if ((p.x>=5*keysize/2) && (p.x<23*keysize/2)) { *j = 5; *i = 1; }
   }
}

int CharOfKeyNo(i,j,keyb)
int i,j;
struct Keyboard *keyb;
{
   int truestate;
   if (j==5) return(' ');
   truestate = keyb->state;
   if (keyb->tempshift) {
     switch (truestate) {
     case LOWERCASE_STATE:
       truestate = UPPERCASE_STATE;
       break;
     case UPPERCASE_STATE:
       truestate = LOWERCASE_STATE;
       break;
     case CONTROL_STATE:
       truestate = CONTROLSHIFT_STATE;
       break;
     case CONTROLSHIFT_STATE:
       truestate = CONTROL_STATE;
       break;
     }
   }  
   if (keyb->tempctrl) {
     switch (truestate) {
     case LOWERCASE_STATE:
       truestate = CONTROL_STATE;
       break;
     case UPPERCASE_STATE:
       truestate = CONTROLSHIFT_STATE;
       break;
     case CONTROL_STATE:
       truestate = LOWERCASE_STATE;
       break;
     case CONTROLSHIFT_STATE:
       truestate = UPPERCASE_STATE;
       break;
     }
   }  
   switch (truestate) {
     case LOWERCASE_STATE:
       return(lowerchars[j-1][i-1]);
       break;
     case UPPERCASE_STATE:
       return(upperchars[j-1][i-1]);
       break;
     case CONTROL_STATE:
       return(lowercontrolchars[j-1][i-1]);
       break;
     case CONTROLSHIFT_STATE:
       return(uppercontrolchars[j-1][i-1]);
       break;
   }
}

int CharOfPos(p,keyb)
Point p;
struct Keyboard *keyb;
{
   int i,j;
   KeyNoOfPos(p,keyb,&i,&j);
   return(CharOfKeyNo(i,j,keyb));
}

KeyOfChar(keyb,ch,key,shift,control)
struct Keyboard *keyb;
int ch;
Rectangle *key, *shift, *control; /* in Keyboard coordinated */
{
   int i,j,sh,ctrl;
   KeyNoOfChar(ch,&i,&j,&sh,&ctrl);
   *key = KeyOfKeyNo(i,j,keyb);
   if (sh) *shift = KeyOfKeyNo(2,4,keyb); else shift = ((Rectangle *) 0);
   if (ctrl) *control = KeyOfKeyNo(1,3,keyb); else control = ((Rectangle *) 0);
}

struct Keyboard *KbdOfPos(p)
Point p;
{
   struct Keyboard *ScanKbds;
   ScanKbds = KbdPool;
   while (ScanKbds!=((struct Keyboard *) 0))
     if (ptinrect(p,raddp(ScanKbds->enclosure,ScanKbds->origin)))
       return(ScanKbds);
     else ScanKbds = ScanKbds->next;
   return(ScanKbds);
}

struct Keyboard *KeyOfPos(p,i,j)
/*
   if p is on a key on some keybord, return that key (i,j) and that Kbd;
   if p is on a keyboard but on no key, return that Kbd and i==j==0;
   if p is on no keyboard, return Kbd==0 and i==j==0.
*/ 
Point p;
int *i, *j;
{
   struct Keyboard *Kbd;
   Kbd = KbdOfPos(p);
   if (Kbd==((struct Keyboard *) 0)) {*i = 0; *j = 0;}
   else KeyNoOfPos(sub(p,Kbd->origin),Kbd,i,j);
   return(Kbd);
}

FlipAllShifts()
{
  struct Keyboard *ScanKbds;
  ScanKbds = KbdPool;
  while (ScanKbds!=((struct Keyboard *) 0)) {
    if (ScanKbds->tempshift) FlipShift(ScanKbds);
    if (ScanKbds->tempctrl) FlipCtrl(ScanKbds);
    ScanKbds = ScanKbds->next;
  }
}



ClearDisplay(r)
Rectangle r;
{
   cursinhibit();
   FrameCurrent();
   FlipAllShifts();
   rectf(&display,r,F_CLR);
   FlipAllShifts();
   FrameCurrent();
   cursallow();
}

ClickKey(ch,keyb)
int ch;
struct Keyboard *keyb;
{
   int sh,ctrl;
   Rectangle key,shift,control;
   KeyOfChar(keyb,ch,&key,&shift,&control);
   if (!((&shift)==((Rectangle *) 0)))
     rectf(&display,inset(raddp(shift,keyb->origin),1),F_XOR);
   if (!((&control)==((Rectangle *) 0)))
     rectf(&display,inset(raddp(control,keyb->origin),1),F_XOR);
   rectf(&display,inset(raddp(key,keyb->origin),1),F_XOR);
   alarm(6);
   wait(ALARM|KBD);
   if (!((&shift)==((Rectangle *) 0)))
     rectf(&display,inset(raddp(shift,keyb->origin),1),F_XOR);
   if (!((&control)==((Rectangle *) 0)))
     rectf(&display,inset(raddp(control,keyb->origin),1),F_XOR);
   rectf(&display,inset(raddp(key,keyb->origin),1),F_XOR);
}

DrawCh(ch,keyb)
int ch;
struct Keyboard *keyb;
{
   Point chbase;
   Rectangle key, shift, control;
   KeyOfChar(keyb,ch,&key,&shift,&control);
   chbase.x = key.origin.x + ((key.corner.x-key.origin.x)/2);
   chbase.y = key.origin.y + KEYMARGIN;
   chbase = add(keyb->origin,chbase);
   DrawTMChar(ch,keyb->font,&display,chbase,keyb);
}

DrawChars(keyb)
struct Keyboard *keyb;
{
   int ch;
   switch (keyb->state) {
     case LOWERCASE_STATE:
       for (ch=NUL; ch<=DEL; ch++)
         if (LowerKey(ch)) DrawCh(ch,keyb);
       break;
     case UPPERCASE_STATE:
       for (ch=NUL; ch<=DEL; ch++)
         if (UpperKey(ch)) DrawCh(ch,keyb);
       break;
     case CONTROL_STATE:
       for (ch=NUL; ch<=DEL; ch++)
         if (CtrlKey(ch)) DrawCh(ch,keyb); 
       break;
     case CONTROLSHIFT_STATE:
       for (ch=NUL; ch<=DEL; ch++)
         if (CtrlShKey(ch)) DrawCh(ch,keyb); 
       break;
   }
}

FlipShift(Kbd)
struct Keyboard *Kbd;
{
   rectf(&display,inset(raddp(KeyOfKeyNo(2,4,Kbd),Kbd->origin),1),F_XOR);
   rectf(&display,inset(raddp(KeyOfKeyNo(13,4,Kbd),Kbd->origin),1),F_XOR);
}

FlipCtrl(Kbd)
struct Keyboard *Kbd;
{
   rectf(&display,inset(raddp(KeyOfKeyNo(1,3,Kbd),Kbd->origin),1),F_XOR);
}

DrawKeyboard(keyb)
struct Keyboard *keyb;
{
   int i,j;
   Rectangle key;
   ClearDisplay(raddp(keyb->enclosure,keyb->origin));
   outline(raddp(keyb->enclosure,keyb->origin));
   for (j=1; j<=5; j++)
   for (i=1; (j==1)?(i<=16):(j==2)?(i<=15):(j<=4)?(i<=14):(i==1); i++) {
     key = KeyOfKeyNo(i,j,keyb);
     outline(inset(raddp(key,keyb->origin),1));
   }
   DrawChars(keyb);
}

FrameCurrent()
{
   if (CurKbd!=((struct Keyboard *) 0)) { 
     outline(inset(raddp(CurKbd->enclosure,CurKbd->origin),1));
     outline(inset(raddp(CurKbd->enclosure,CurKbd->origin),2));
   }
}

Current(Kbd)
struct Keyboard *Kbd;
{
   if (CurKbd != Kbd) {
     FrameCurrent();
     CurKbd = Kbd;
     FrameCurrent();
   }
}

Point MoveFrame (r)
/* given a rectangle r (screen coord) and a point p (screen coord)
   tracks while p moves by Dp and returns the origin of r moved by Dp */
Rectangle r;
{
   Point initp,oldp,p;
   wait(MOUSE);
   p = mouse.xy;
   initp = p;
   oldp = p;
   border(r);
   while (!button123()) {
     wait(CPU);
     wait(MOUSE);
     p = mouse.xy;
     if (!eqpt(p,oldp)) {
       border(raddp(r,sub(oldp,initp)));
       border(raddp(r,sub(p,initp)));
       oldp = p;
     }
   }
   border(raddp(r,sub(p,initp)));
   while (button123()) { wait(CPU); wait(MOUSE); }
   return(sub(p,sub(initp,r.origin)));
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
     } else {
       if (s-t<50) {
         *s++ = (str[0] = c);
         p = string(&defont,str,&display,p,F_XOR);
       }
     }
   }
}

ReDr(Kbd)
struct Keyboard *Kbd;
{
   if (Kbd==((struct Keyboard *) 0)) {}
   else {
     ReDr(Kbd->next);
     DrawKeyboard(Kbd);
   }
}

KbdReDraw()
{
   ReDr(KbdPool);
}


SubKbd(Kbd)
struct Keyboard *Kbd;
{
   struct Keyboard *ScanKbds;
   if (Kbd!=((struct Keyboard *) 0)) {
     if (KbdPool==((struct Keyboard *) 0)) {}
     else if (KbdPool==Kbd) KbdPool = KbdPool->next;
     else {
       ScanKbds = KbdPool;
       while (ScanKbds != ((struct Keyboard *) 0))
       if (ScanKbds->next == Kbd) ScanKbds->next = Kbd->next;
       else ScanKbds = ScanKbds->next;
     }
   }
}

TopKbd(Kbd)
struct Keyboard *Kbd;
{
   SubKbd(Kbd);
   Kbd->next = KbdPool;
   KbdPool = Kbd;
}

DelKbd(Kbd)
struct Keyboard *Kbd;
{
   SubKbd(Kbd);
   ffree(Kbd->font);
   free(Kbd);
   Kbd = ((struct Keyboard *) 0);
}
/*
ClipOnKbds(r,Kbd)
Rectangle r;
struct Keyboard *Kbd;
{
   struct Keyboard *ScanKbds;
   ScanKbds = KbdPool;
   while (ScanKbds!=((struct Keyboard *) 0)) {
     if (ScanKbds!=Kbd) 
       rectclip(r,raddp(ScanKbds->enclosure,ScanKbds->origin));
   }
}
*/
int KbdKeyStroke(font)
Font *(*font);
{
  int ch;
  ch = kbdchar();
  if ((ch<128) && (CurKbd!=((struct Keyboard *) 0))) {
    ClickKey(ch,CurKbd);
    *font = CurKbd->font;
  } else *font = ((Font *) 0);
  return(ch);
}

FlipKey(i,j,Kbd,cursor)
int i,j;
struct Keyboard *Kbd;
Point cursor;
{
   Rectangle key;
   if (Kbd!=((struct Keyboard *) 0)) {
     if ((i==0) || (j==0)) Current(Kbd);
     else {
       key = KeyOfKeyNo(i,j,Kbd);
       rectf(&display,inset(raddp(key,Kbd->origin),1),F_XOR);
       KbdDrawChar(CharOfKeyNo(i,j,Kbd),Kbd->font,&display,cursor);
     }
   }
}

int KbdButt1(font,cursor)
Font *(*font);
Point cursor;
{
   int i,j,oldi,oldj;
   int ch;
   Rectangle key;
   Point p;
   struct Keyboard *Kbd, *OldKbd;
   ch = 0; *font = ((Font *) 0);
   Kbd = KeyOfPos(mouse.xy,&i,&j);
   if ((Kbd!=(struct Keyboard *) 0) && (Kbd==CurKbd) &&
       (!LocalTop(Kbd)) && ((i==0) || (j==0))) {
     TopKbd(Kbd);
     DrawKeyboard(Kbd);
     Current(Kbd);
   }
   oldi = i; oldj = j; OldKbd = Kbd;
   FlipKey(i,j,Kbd,cursor);
   while (button1()) {
     wait(CPU);
     wait(MOUSE);
     Kbd = KeyOfPos(mouse.xy,&i,&j);
     if ((i!=oldi) || (j!=oldj) || (Kbd!=OldKbd)) {
       FlipKey(oldi,oldj,OldKbd,cursor);
       FlipKey(i,j,Kbd,cursor);
       oldi = i; oldj = j; OldKbd = Kbd;
     }
   }
   FlipKey(i,j,Kbd,cursor);
   if (Kbd!=((struct Keyboard *) 0)) {
     if ((i==0) || (j==0)) Current(Kbd);
     else {
       if (((i==2) || (i==13)) && (j==4)) { /* shift keys */
         Kbd->tempshift = !(Kbd->tempshift);
         FlipShift(Kbd);
       } else if ((i==1) && (j==3)) { /* ctrl key */
         Kbd->tempctrl = !(Kbd->tempctrl);
         FlipCtrl(Kbd);
       } else { 
         ch = CharOfKeyNo(i,j,Kbd);
         *font = Kbd->font;
         if (Kbd->tempshift) FlipShift(Kbd);
         Kbd->tempshift = FALSE;
         if (Kbd->tempctrl) FlipCtrl(Kbd);
         Kbd->tempctrl = FALSE;
       }
     }
   }
   return(ch);
}

Font *GetFont()
{
   char FNAME[50];
   Point p;
   Font *font;
   font = ((Font *) 0);
   while (font==((Font *)0)) {
     p = string(&defont,"Font: ",&display,
                Pt(Drect.origin.x+10,Drect.corner.y-15),F_XOR);
     getstr(FNAME,p);
     p = string(&defont,"Font: ",&display,
                Pt(Drect.origin.x+10,Drect.corner.y-15),F_XOR);
     string(&defont,FNAME,&display,p,F_XOR);
     if (!FNAME[0]) return(font);
     else font = getfont(FNAME);
   }
   return(font);
}

KbdNew()
{
   Point center;
   Font *font;
   font = GetFont();
   if (font != ((Font *) 0)) {
     KbdPool = MakeKeyboard(font,Pt(0,0),KbdPool);
     center = div(sub(KbdPool->enclosure.corner,KbdPool->enclosure.origin),2);
     KbdPool->origin =
       sub(MoveFrame(raddp(KbdPool->enclosure,sub(mouse.xy,center))),
           KbdPool->enclosure.origin);
     DrawKeyboard(KbdPool);
     Current(KbdPool);
   }
}

IntersOther(Kbd)
struct Keyboard *Kbd;
{
   struct Keyboard *ScanKbds;
   ScanKbds = KbdPool;
   while (ScanKbds!=((struct Keyboard *) 0)) {
     if (ScanKbds!=Kbd)
       if (rectXrect(raddp(Kbd->enclosure,Kbd->origin),
                     raddp(ScanKbds->enclosure,ScanKbds->origin)))
         return(TRUE);
     ScanKbds = ScanKbds->next;
   }
   return(FALSE);
}

bool LocalTop(Kbd)
struct Keyboard *Kbd;
{
   struct Keyboard *ScanKbds;
   ScanKbds = KbdPool;
   while (ScanKbds!=Kbd) {
       if (rectXrect(raddp(Kbd->enclosure,Kbd->origin),
                     raddp(ScanKbds->enclosure,ScanKbds->origin)))
         return(FALSE);
     ScanKbds = ScanKbds->next;
   }
   return(TRUE);
}

KbdMove()
{
   int Intersects;
   Point p;
   struct Keyboard *Kbd;
   Kbd = KbdOfPos(mouse.xy);
   if (Kbd!=((struct Keyboard *) 0)) {
     Intersects = IntersOther(Kbd);
     p = MoveFrame(raddp(Kbd->enclosure,Kbd->origin));
     ClearDisplay(raddp(Kbd->enclosure,Kbd->origin));
     if (Kbd==CurKbd) FrameCurrent();
     if (Kbd->tempshift) FlipShift(Kbd);
     if (Kbd->tempctrl) FlipCtrl(Kbd);
     Kbd->origin = sub(p,Kbd->enclosure.origin);
     if (Kbd->tempshift) FlipShift(Kbd);
     if (Kbd->tempctrl) FlipCtrl(Kbd);
     if (Kbd==CurKbd) FrameCurrent();
     TopKbd(Kbd);
     if (Intersects) KbdReDraw();
     else DrawKeyboard(Kbd);
   }
}

KbdDelete()
{
   int Intersects;
   struct Keyboard *Kbd;
   Kbd = KbdOfPos(mouse.xy);
   if (Kbd!=((struct Keyboard *) 0)) {
     /* ***** confirm */
     Intersects = IntersOther(Kbd);
     if (Kbd==CurKbd) Current((struct Keyboard *) 0);
     ClearDisplay(raddp(Kbd->enclosure,Kbd->origin));
     if (Kbd->tempshift) {FlipShift(Kbd); Kbd->tempshift = FALSE;}
     if (Kbd->tempctrl) {FlipCtrl(Kbd); Kbd->tempctrl = FALSE;}
     DelKbd(Kbd);
     if (Intersects) KbdReDraw();
     if ((KbdPool!=((struct Keyboard *) 0)) && 
         (KbdPool->next==((struct Keyboard *) 0)) &&
         (CurKbd!=KbdPool))
       Current(KbdPool);
   }
}

KbdInit()
{
   KbdPool = ((struct Keyboard *) 0);
   CurKbd = ((struct Keyboard *) 0);
   fontcounter = 1;

}
