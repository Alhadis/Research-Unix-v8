
#include <jerq.h>

#define PEN_DRAW_STOP	0
#define PEN_CLEANUP	1
#define PEN_EXIT	2

Bitmap screen;

char *menutext[] =
  {"    ","clean up","exit",NULL};

Menu menu = {menutext};

Point midlayer,realcurs,fakecurs,oldfakecurs;

int drawing;

Texture grey = {
  0x1111, 0x4444, 0x1111, 0x4444,
  0x1111, 0x4444, 0x1111, 0x4444,
  0x1111, 0x4444, 0x1111, 0x4444,
  0x1111, 0x4444, 0x1111, 0x4444,
};

CleanUp()
{
  texture(&screen,Rect(0,0,XMAX-9,8),&grey,F_STORE);
  texture(&screen,Rect(XMAX-9,0,XMAX,YMAX-9),&grey,F_STORE);
  texture(&screen,Rect(8,YMAX-9,XMAX,YMAX),&grey,F_STORE);
  texture(&screen,Rect(0,8,8,YMAX),&grey,F_STORE);
}

PenTract(p,q)
Point p,q;
{
   segment(&screen,p,q,F_OR);
   segment(&screen,add(p,Pt(1,0)),add(q,Pt(1,0)),F_OR);
   segment(&screen,add(p,Pt(0,1)),add(q,Pt(0,1)),F_OR);
   segment(&screen,add(p,Pt(1,1)),add(q,Pt(1,1)),F_OR);
}

PenPoint(p)
Point p;
{
   rectf(&screen,Rect(p.x,p.y,p.x+2,p.y+2),F_XOR);
}

main()
{
   screen.base = addr(&display,Pt(0,0));
   screen.width = 50;
   screen.rect = Jrect;
   request(MOUSE);
   midlayer = div(add(Drect.origin,Drect.corner),2);
   wait(MOUSE);
   fakecurs = mouse.xy;
   PenPoint(fakecurs);
   cursset(midlayer); nap(2);
   drawing = 1;
   cursinhibit();
   menutext[0]="stop";
   for (;;) {
     wait(MOUSE);
     if (drawing) {
       realcurs = mouse.xy;
       cursset(midlayer); nap(2);
       oldfakecurs = fakecurs;
       fakecurs = add(fakecurs,div(sub(realcurs,midlayer),2));
       if (!eqpt(fakecurs,oldfakecurs)) {
         PenPoint(oldfakecurs);
         if (button1()) PenTract(oldfakecurs,fakecurs);
         PenPoint(fakecurs);
       }
     }
     if button3() {
       if (drawing) cursallow();
       switch(menuhit(&menu,3)) {
         case PEN_DRAW_STOP:
           if (drawing) {
             menutext[0]="draw";
             drawing = 0;
           } else {
             midlayer = div(add(Drect.origin,Drect.corner),2); /* reshape ... */
             menutext[0]="stop";
             drawing = 1;
           }
           break;
         case PEN_CLEANUP:
           CleanUp();
           break;
         case PEN_EXIT:
           exit();
         default: {}
       }
       if (drawing) cursinhibit();
     }
   }
}
