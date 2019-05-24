/*
 * simulate a connector looped back on itself
 */
#include <stdio.h>

main()
{
	int c;
	system("68ld new");
	system("stty raw -echo");
	while ((c = getch()) != 4)
		write(1,&c,1);
	system("stty -raw echo");
}

getch()
{
	int c = 0;
	read(0,&c,1);
	return(c);
}
