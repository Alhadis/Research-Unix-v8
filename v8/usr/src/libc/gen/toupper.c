int
toupper(c)
register int c;
{
	if(c >= 'a' && c <= 'z')
		c += 'A' - 'a';
	return(c);
}
