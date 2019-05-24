main()
{
	char s[100];
	while(gets(s)) 
		if(access(s,04)==0 && access(s,01)==-1)
			puts(s);
}
