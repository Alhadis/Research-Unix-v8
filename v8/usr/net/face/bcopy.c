bcopy(s1, s2, d)
	register char *s1, *s2;
	register char *d;
{
	register long n=s2-s1;
#ifdef	vax
	/*   movc3	 n, s1, d */
	asm("movc3	r8,(r11),(r9)");
#else
	if(n>0) do
		*d++ = *s1++;
	while(--n);
#endif
}
