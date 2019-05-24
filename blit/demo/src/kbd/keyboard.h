
#define LOWERCASE_STATE       1
#define UPPERCASE_STATE       2
#define CONTROL_STATE         3
#define CONTROLSHIFT_STATE    4

typedef struct Keyboard {
  Point origin;
  Rectangle enclosure,key;
  int keysize;
  int state, tempshift, tempctrl;
  int fontcode;
  Font *font;
  struct Keyboard *next;
};

extern struct Keyboard *KbdPool;
   /* list of existing keyboards */
extern KbdInit();
   /* to be called once before any of the other routines */
extern int KbdKeyStroke(/* Font *(*font) */);
   /* returns the next character typed (assuming own()&KBD) and its font
      (called by ref); *font is ((Font *)0) if there is no active keyboard or
      if the char is non-ASCII */
extern int KbdButt1(/* Font *(*font), Point cursor */);
   /* returns the keyboard character pointed by button1 (assuming button1())
      and its font (called by ref); *font is ((Font *)0) if button1 pointed
      at no char. The cursor is used to display the character while button1
      is depressed. */
extern KbdChangeState(/* int newstate, struct Keyboard *Kbd */);
   /* Change the state of a keyboard to newstate. Keyboard states are
       LOWERCASE_STATE, UPPERCASE_STATE, CONTROL_STATE and CONTROLSHIFT_STATE */
extern struct Keyboard *KbdOfPos(/* Point p */);
   /* Returns the topmost keyboard at point p.
      Returns ((Keyboard *) 0) if there is no keyboard at p. */
extern KbdNew();
   /* Create a keyboard */
extern KbdMove();
   /* Move a keyboard */
extern KbdDelete();
   /* Delete a keyboard */
extern KbdDrawChar (/* char c, Font *fp, Bitmap *db, Point p */);
  /* draw a char with reference point at p */
