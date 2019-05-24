#include <jerq.h>
#include <jerqio.h>
Word stagebits[XMAX>>WORDSHIFT];	/* width of screen; better be enough! */
Bitmap stage={
	stagebits,
	XMAX>>WORDSHIFT,
	0, 0, XMAX, 1
};
Bitmap *
rdbitmap(f)
	char *f;
{
	register FILE *fd=fopen(f, "r");
	register Bitmap *b=0;
	Rectangle r;
	register y, n;
	if(fd==0){
		msg("can't find file");
		return 0;
	}
	if(fread((char *)&r, sizeof r, 1, fd)!=1){
		msg("file not bitmap");
		goto Return;
	}
	if((b=balloc(r))==0){
		msg("no bitmap space in jerq");
		goto Return;
	}
	n=r.corner.x-r.origin.x;
	for(y=r.origin.y; y<r.corner.y; y++){
		if(fread((char *)stagebits, (n+7)/8, 1, fd) != 1){
			msg("short file, returning it anyway");
			break;
		}
		bitblt(&stage, Rect(0, 0, n, 1), b, Pt(r.origin.x, y), F_STORE);	
	}
 Return:
	fclose(fd);
	return b;
}
/*
 * Write rectangle r from bitmap b to file f.
 * p is the lockstep position, origin of bitmap such that
 * textures work.  if you don't care about texture alignment,
 * use b->rect.origin
 */
wrbitmap(b, p, r, f)
	register Bitmap *b;
	Point p;
	Rectangle r;
	char *f;
{
	register FILE *fd=fopen(f, "w");
	register x, y, n;
	x=r.origin.x;
	r=rsubp(r, p);
	if(fd==0 || fwrite(&r, sizeof r, 1, fd)!=1){
		msg("can't create file");
		return 0;
	}
	r=raddp(r, p);
	n=r.corner.x-r.origin.x;
	for(y=r.origin.y; y<r.corner.y; y++){
		bitblt(b, Rect(x, y, x+n, y+1), &stage, Pt(0, 0), F_STORE);
		if(fwrite((char *)stagebits, (n+7)/8, 1, fd) != 1){
			msg("write error");
			fclose(fd);
			return 0;
		}
	}
	fclose(fd);
	return 1;
}
