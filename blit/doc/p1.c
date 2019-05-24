#include <blit.h>
#define	XSPACING	25
#define	YSPACING	25
main(){
	Point p;
	/* Vertical lines */
	for(p.x=p.y=0; p.x<XMAX; p.x+=XSPACING)
		jsegment(p, Pt(p.x, YMAX), F_OR);
	/* Horizontal lines */
	for(p.x=p.y=0; p.y<YMAX; p.y+=YSPACING)
		jsegment(p, Pt(XMAX, p.y), F_OR);
}
