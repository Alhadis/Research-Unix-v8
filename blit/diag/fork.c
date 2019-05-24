#include <stdio.h>
FILE *
Fopen(file, suf, mode)
	char *file, *suf, *mode;
{
	FILE *f;
	char buf[100];
	sprintf(buf, "%s%s", file, suf);
	f=fopen(buf, mode);
	if(f==NULL){
		fprintf("fork: can't open %s\n", buf);
		exit(1);
	}
	return f;
}
main(argc, argv)
	char *argv[];
{
	char buf[256];
	FILE *in, *out1, *out2;
	register c;
	if(argc!=2){
		fprintf(stderr, "usage: fork file\n");
		exit(1);
	}
	in=Fopen(argv[1], "", "r");
	out1=Fopen(argv[1], ".0", "w");
	out2=Fopen(argv[1], ".1", "w");
	while(fgets(buf, 256, in), !feof(in)){
		fputs(buf, out1);
		fgets(buf, 256, in);
		fputs(buf, out2);
	}
	fclose(out1);
	fclose(out2);
	return 0;
}
