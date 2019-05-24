#include <jerq.h>
#include <font.h>
#define WID	10	/* logical width of maze */
#define HT	6	/* logical height of maze */
#define EAST	0
#define NORTH	1
#define WEST	2
#define SOUTH	3

#define W	700	/* width of perspective viewing area */
#define H	600	/* height of perspective viewing area */
#define LEFT	((XMAX - W) >> 1)
#define RIGHT	(XMAX - LEFT)
#define	UP	(LEFT)
#define DOWN	(LEFT + H)
#define MIDDLE	400
#define	N	16
int XSIZE,YSIZE;	/* actual height of our window */
#define	NAMSIZE	10
int ME;		/* my index, max index */
typedef struct State {
	char id;	/* is he playing? */
	char dir;	/* and direction */
	short score;
	char *pos;	/* pointer to maze position */
	char vis, ovis;	/* visibility flag */
	char name[NAMSIZE];	/* goddam string allocation nonsense */
} State;
State *me,player[N];
extern Point delta[];		/* movements for top view */
extern int dx[],dx1[],dy[],dreal[];	/* increments for perspective view */
extern char inc[],forw[],right[],left[];

extern State *getplayer();
