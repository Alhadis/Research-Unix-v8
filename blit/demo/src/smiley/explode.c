#include <jerq.h>
#define	MAXBOOMS	10
#define	MAXSHARDS	30
#define	DX		4	/* Power of 2 */
#define	TIMELEFT	30	/* Number of clock ticks before fade */
struct boom{
	int	active;
	struct shard{
		int	x;
		int	y;
		int	dx;
		int	dy;
		int	timeleft;
	}s[MAXSHARDS];
}boom[MAXBOOMS];
exstart(x, y)
	register x, y;
{
	register struct boom *b;
	register struct shard *s;
	for(b=boom; b<&boom[MAXBOOMS]; b++)
		if(b->active==0){
			b->active++;
			for(s=b->s; s<&b->s[MAXSHARDS]; s++){
				s->x=_max(x, 1);
				s->x=_min(x, XMAX-2);
				s->y=_max(y, 1);
				s->y=_min(y, YMAX-2);
				s->dx=((rand()>>8)&(2*DX-1))-DX;
				s->dy=((rand()>>8)&(2*DX-1))-DX;
				s->timeleft=(TIMELEFT/2)+(rand()>>8)%TIMELEFT;
			}
			return;
		}
}
exmove(){
	register struct boom *b;
	register struct shard *s;
	register shardalive;
	register boomalive;
	boomalive=0;
	for(b=boom; b<&boom[MAXBOOMS]; b++)
		if(b->active){
			shardalive=0;
			for(s=b->s; s<&b->s[MAXSHARDS]; s++){
				if(s->timeleft==0)
					continue;
				if(--s->timeleft<=0)
					exblob(s->x, s->y, 0);
				else{
					shardalive++;
					exblob(s->x, s->y, 0);
					s->x+=s->dx;
					s->y+=s->dy;
					if(s->x>=1 && s->x<XMAX-1 && s->y>=1 && s->y<YMAX-1)
						exblob(s->x, s->y, 1);
					else
						s->timeleft=1;	/* get it next time */
				}
			}
			if(shardalive==0)
				exclear(b);
			else
				boomalive++;
		}
	return(boomalive);
}
exinit(){
	register struct boom *b;
	for(b=boom; b<&boom[MAXBOOMS]; b++)
		exclear(b);
}
exclear(b)
	register struct boom *b;
{
	register struct shard *s;
	if(b->active){
		for(s=b->s; s<&b->s[MAXSHARDS]; s++)
			if(s->timeleft)
				exblob(s->x, s->y, 0);
		b->active=0;
	}
}
exblob(x, y, bit)
	register x;
{
	jpoint(Pt(x, y), F_XOR);
}
_min(a, b){
	return(a<b? a : b);
}
_max(a, b){
	return(a>b? a : b);
}
