sqrt(x)
	register long x;
{	register short s;
	register long n, u;
	if(x<=0)
		return 0;
	for(s = 2, n = 4; n < x; s += s, n *= 4)
		;
loop:
	u = (x + (long)s * s)/s;
	u >>= 1;
	if(u >= s)
		return(s);
	s = u;
	goto loop;
}
