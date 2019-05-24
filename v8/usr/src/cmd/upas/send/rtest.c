#include <stdio.h>

char * thissys = "THISSYS";

main()
{
	char buf[132];
	char cmd[132];
	char format[132];
	char sysname[132];

	getrules();
	putchar('>');
	fflush(stdout);
	while(gets(buf)) {
		switch(rewrite(buf, "USER", 1, cmd, format, sysname)) {
		case -1:
			printf("can't recognize %s\n", buf);
			break;
		case 1:
			printf("can't forward %s\n", buf);
		case 0:
			printf("%s->'%s'\n", buf, cmd);
			break;
		}
		putchar('>');
		fflush(stdout);
	}
}
