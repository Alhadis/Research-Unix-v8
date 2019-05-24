#include <sys/inet/in.h>

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
char *
in_ntoa(in)
in_addr in;
{
	static char b[18];
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	sprintf(b, "%d.%d.%d.%d", UC(p[3]), UC(p[2]), UC(p[1]), UC(p[0]));
	return (b);
}

/*
 * Convert base 256 d.d.d.d representation
 * to network-format internet address.
 */
in_addr
in_aton(s)
char *s;
{
	int x;
	char *p, c4[4];

	for(x = 0; x < 4; x++)
		c4[x] = 0;
	p = s;
	x = 0;
	while(*p){
		if(p == s || *p == '.'){
			if(*p == '.')
				p++;
			x <<= 8;
			x |= atoi(p);
		}
		p++;
	}
	if((x & 0xffff0000) == 0){
		/* probably like 126.1 */
		x = ((x & 0xff00) << 16) | (x & 0xff);
	}
	return(x);
}
