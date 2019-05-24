#include "alph.h"
int nums[1024], done;
frac_r (n, a, b)
{
	int v;

	v = nums[n] % (b-a+1);
	if (v<0)
		return(v+b+1);
	else
		return(v+a);
}
#define	DITHSIZE	8
#define	DITHMASK	(DITHSIZE-1)
int dith[DITHSIZE][DITHSIZE]={
	0,	32,	8,	40,	2,	34,	10,	42,
	48,	16,	56,	24,	50,	18,	58,	26,
	12,	44,	4,	36,	14,	46,	6,	38,
	60,	28,	52,	20,	62,	30,	54,	22,
	3,	35,	11,	43,	1,	33,	9,	41,
	51,	19,	59,	27,	49,	17,	57,	25,
	15,	47,	7,	39,	13,	45,	5,	37,
	63,	31,	55,	23,	61,	29,	53,	21,
};

surf (x0, y0, size, h0, n0, h1, n1, h2, n2, h3, n3)
{
	int h01, n01, h12, n12, h23, n23, h30, n30, hc, nc;

	if (!done) {
		done=keyhit();
		if (size==1) {
			if (dith[x0&DITHMASK][y0&DITHMASK] > (h0+h1+h2+h3)/64+32)
				*(display.base+((y0)*(unsigned)50)+((x0)>>4))|=1<<(15-((x0)&15));
			else
				*(display.base+((y0)*(unsigned)50)+((x0)>>4))&=~(1<<(15-((x0)&15)));
		} else {
			n01=(n0+n1) % 1024;
			h01=(h0+h1) / 2+frac_r(n01, -size, size);
			n12=(n1+n2) % 1024;
			h12=(h1+h2) / 2+frac_r(n12, -size, size);
			n23=(n2+n3) % 1024;
			h23=(h2+h3) / 2+frac_r(n23, -size, size);
			n30=(n3+n0) % 1024;
			h30=(h3+h0) / 2+frac_r(n30, -size, size);
			nc=(n0+n1+n2+n3)% 1024;
			hc=(h01+h12+h23+h30) / 4+frac_r(nc, -size, size);
			size=size / 2;
			surf(x0     , y0     , size, h0, n0, h01, n01, hc, nc, h30, n30);
			surf(x0+size, y0     , size, h01, n01, h1, n1, h12, n12, hc, nc);
			surf(x0+size, y0+size, size, hc, nc, h12, n12, h2, n2, h23, n23);
			surf(x0     , y0+size, size, h30, n30, hc, nc, h23, n23, h3, n3);
		}
	}
}

fractal() {
	int i;

	clearscreen();
	done=false;
	while (!done) {
		for (i=0; i<1024; i++) nums[i]=random();
		surf((XMAX-512)/2, (YMAX-512)/2, 512, 0, 0, 0, 1, 0, 2, 0, 3);
	}
}
