
#include <jerq.h>
#include <font.h>
#include "keyboard.h"

#define KBD_NEW    0
#define KBD_MOVE   1
#define KBD_DELETE 2
#define KBD_EXIT   3
char *menutext[] = {"new", "move", "delete", "exit", NULL};
Menu mainmenu = {menutext};
   
#define LOCK_LOWER         0
#define LOCK_UPPER         1
#define LOCK_CONTROL       2
#define LOCK_CONTROLSHIFT  3
char *locktext[] = {"lower case", "upper case", "control", "control shift", NULL};
Menu lockmenu = {locktext};

Rectangle textspace;

Point AdvanceCursor(p, c, fp)
   /* Given a cursor, a character and a font, moves the cursor horizontally
      of the width of that character in that font. */
Point p;
char c;
Font *fp;
{
   Point q;
   if (c<=fp->n) {
     q.x = p.x + (((fp->info)[c]).width);
     q.y = p.y;  
     if ((q.x+50)>(textspace.corner.x)) {
       rectf(&display,textspace,F_CLR);
       q.x = Drect.origin.x + 50;
     }
     return(q);
   } else return(p);
}

KbdButt2()
{
   struct Keyboard *Kbd;
   Kbd = KbdOfPos(mouse.xy);
   if (Kbd!=((struct Keyboard *)0)) {
     switch (menuhit(&lockmenu,2)) {
       case LOCK_LOWER:
         KbdChangeState(LOWERCASE_STATE,Kbd);
         break;
       case LOCK_UPPER:
         KbdChangeState(UPPERCASE_STATE,Kbd);
         break;
       case LOCK_CONTROL:
         KbdChangeState(CONTROL_STATE,Kbd);
         break;
       case LOCK_CONTROLSHIFT:
         KbdChangeState(CONTROLSHIFT_STATE,Kbd);
         break;
     }
   }
}

KbdButt3()
{
   switch (menuhit(&mainmenu,3)) {
     case KBD_NEW:
       KbdNew();
       break;
     case KBD_MOVE:
       KbdMove();
       break;
     case KBD_DELETE:
       KbdDelete();
       break;
     case KBD_EXIT:
       exit();
   }
}

Execute()
{
   int ch;
   Point cursor;
   Font *font;

   KbdInit();

   textspace.origin.x = Drect.origin.x+50;
   textspace.origin.y = Drect.origin.y;
   textspace.corner.x = Drect.corner.x-50;
   textspace.corner.y = Drect.origin.y + 100;

   cursor.x = textspace.origin.x;
   cursor.y = textspace.corner.y-20;

   for (;;) {
     wait(CPU);
     if (own() & KBD) {
       ch = KbdKeyStroke(&font);
       if (font != ((Font *) 0)) {
         KbdDrawChar(ch,font,&display,cursor);
         cursor = AdvanceCursor(cursor,ch,font);
       }
     }
     if (own() & MOUSE) 
       if (button123()) 
         if (button1()) {
           ch = KbdButt1(&font,cursor);
           if (font != ((Font *) 0)) {
             KbdDrawChar(ch,font,&display,cursor);
             cursor = AdvanceCursor(cursor,ch,font);
           }
         } else if (button2()) KbdButt2();
         else if (button3()) KbdButt3();
   }
}

main()
{

   jinit();

   request(MOUSE|KBD|SEND|RCV);

   Execute();

}
