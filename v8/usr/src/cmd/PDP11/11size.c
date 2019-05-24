#include	<stdio.h>
#include 	"a.out.h"		/*god: using pdp 11 format*/

/*
	size -- determine object size

*/

int a_magic[] = {A_MAGIC1,A_MAGIC2,A_MAGIC3,A_MAGIC4,0};

main(argc, argv)
char **argv;
{
	struct exec buf;
	long sum;
	int gorp,i;
	FILE *f;

	if (argc==1) {
		*argv = A_DOT_OUT;	/*god*/
		argc++;
		--argv;
	}
	gorp = argc;
	while(--argc) {
		++argv;
		if ((f = fopen(*argv, "r"))==NULL) {
			printf("11size: %s not found\n", *argv);
			continue;
		}
		fread((char *)&buf, sizeof(buf), 1, f);
		for(i=0;a_magic[i];i++)
			if(a_magic[i] == buf.a_magic) break;
		if(a_magic[i] == 0) {
			printf("11size: %s not an object file\n", *argv);
			fclose(f);
			continue;
		}
		if (gorp>2)
			printf("%s: ", *argv);
		printf("%u+%u+%u = ", buf.a_text,buf.a_data,buf.a_bss);
		sum = (long) buf.a_text + (long) buf.a_data + (long) buf.a_bss;
		printf("%ldb = %#lob\n", sum, sum);
		fclose(f);
	}
}
