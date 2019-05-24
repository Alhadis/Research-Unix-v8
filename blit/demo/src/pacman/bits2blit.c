#include <stdio.h>
extern unsigned char chr24[][72];
extern unsigned char chr40[][200];

unsigned short converter[4000];

#define N24	10
#define N40	26	/* Make sure that these are even, or else nothing works */

int i,j,k;
char *next;
unsigned short *pnext;

unsigned swab(x)
unsigned x;
{
	return(((x<<8)&0xff00)+((x>>8)&0xff));
}

main()
{
	printf("#include <jerq.h>\n\n");
	printf("Word Bits24[] = {\n");
	
	next = (char *)(pnext = converter);

	for(i=0;i<24;i++)
		for(j=0;j<N24;j++)
			for(k=0;k<3;k++)
				*next++ = chr24[j][3*i+k];

	for(i=0;i<36;i++) {
		printf("\n\t");
		for(j=0;j<N24;j++) {
			printf("0x%x",swab(*pnext++));
			if((j==N24-1)&&(i==36-1)) printf("\n");
			else if((j%6)==5) printf(" ,\n\t");
			else printf(" , ");
		}
		printf("\n");
	}

	printf("};\n\n");

	printf("Word Bits40[] = {\n");
	
	next = (char *)(pnext = converter);

	for(i=0;i<40;i++)
		for(j=0;j<N40;j++)
			for(k=0;k<5;k++)
				*next++ = chr40[j][5*i+k];

	for(i=0;i<100;i++) {
		printf("\n\t");
		for(j=0;j<N40;j++) {
			printf("0x%x",swab(*pnext++));
			if((j==N40-1)&&(i==100-1)) printf("\n");
			else if((j%5)==4) printf(" ,\n\t");
			else printf(" , ");
		}
		printf("\n");
	}

	printf("};\n\n");

	printf("Bitmap Bitmap24 = { Bits24,%d, 0,0,%d,%d ,NULL };\n\n",3*(N24/2),N24*24,24);

	printf("Bitmap Bitmap40 = { Bits40,%d, 0,0,%d,%d ,NULL };\n\n",5*(N40/2),N40*40,40);

}
