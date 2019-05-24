#include <blit.h>
Point spot, v, a;
int Topx, Topy;
int Botx, Boty;
Bitmap *ball;
#define	R	32
main()
{
	Topx=Drect.corner.x-R;
	Topy=Drect.corner.y-R;
	Botx=Drect.origin.x+R;
	Boty=Drect.origin.y+R;
	a.x = 0; a.y = 1;
	spot.x = (Botx+Topx)/2;	
	spot.y = Boty;
	/* allocate a bitmap */
	ball=balloc(Rect(0, 0, 2*R+1, 2*R+1));
	rectf(ball, ball->rect, F_CLR);
	/* draw ball as a disc */
	disc(ball, add(ball->rect.origin, Pt(R, R)), R, F_XOR);
	drawball();
	v.x=1; v.y=0;
	for(;;){
		drawball();	/* undraw the old one */
		v = add(v, a);
		spot = add(spot, v);
		if(spot.x >= Topx) {
			spot.x = 2*Topx - spot.x;
			v.x = -v.x;
		} else if(spot.x <= Botx) {
			spot.x = 2*Botx - spot.x;
			v.x = -v.x;
		}
		if(spot.y >= Topy) {
			spot.y = 2*Topy - spot.y;
			v.y = -v.y;
		} else if(spot.y <= Boty) {
			spot.y = 2*Boty - spot.y;
			v.y = -v.y;
		}
		drawball();	/* draw the new one */
		sleep(4);
	}
}
drawball(){
	bitblt(ball, ball->rect, &display, sub(spot, Pt(R, R)), F_XOR);
}
