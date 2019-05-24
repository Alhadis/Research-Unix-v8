/* To hell with it, I say.  This is pure Jerqdom, no ifdef's -- rob */
#include <jerq.h>
#define true 1
#define false 0
#define abs(p) (((p)<0) ? -(p) : (p))

#define	REPLACE	F_STORE
#define	NOT	F_XOR	/* Wrong */
#define	AND	F_XOR	/* Wrong */
#define	ANDNOT	F_CLR
#define	OR	F_OR
#define	ORNOT	F_XOR
#define	XOR	F_XOR
#define	XORNOT	XOR
#define	SET	F_STORE
#define	CLEAR	F_CLR

char c;
int param, hitkey;
#define	rasterop(f, w, h, dx, dy, dll, dmp, sx, sy, sll, smp) bitblt(&display, Rect(sx, sy, sx+w-1, sy+h-1), &display, Pt(dx, dy), f)
#define	rasterop1(a, b, c, d) rectf(&display, Rect(c, d, a+c, b+d), GXfunction)
int	GXfunction;
