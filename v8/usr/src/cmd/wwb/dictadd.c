/* NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT */
/* Writer's Workbench version 2.1, March 8, 1981 */
#include <stdio.h>
main(argc,argv)
char *argv[];
int argc;
{
	FILE *fp, *fopen();
	int len;
	char phrase[100];
	char *ptr = phrase;
	if((fp=fopen(argv[1],"a"))==NULL){
		fprintf(stderr,"Dictadd can't write on %s\n",argv[1]);
		exit(1);
	}
	putchar('>');
	while(gets(phrase) != EOF){
		len = strlen(phrase);
		if((*ptr == 'q' || *ptr == '.') && len == 1)break;
		if(*ptr=='\0' ) {
			fprintf(stderr,"error: pattern not stored\n");
			continue;
		}
		if(*ptr!='~'&& *ptr!=' ')putc(' ',fp);
		fprintf(fp,"%s",phrase);
		if(*ptr!='~'&& phrase[len-1]!=' ') putc(' ',fp);
		putc('\n',fp);
		putchar('>');
	}
}
