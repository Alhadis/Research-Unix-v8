#include <stdio.h>
main()
{
	char lines[40][41];
	int i,j;
	for ( i=0 ; i<40 ; i++ )
		gets(lines[i]);
	for ( i=0 ; i<40 ; i++ ) {
		for ( j=0 ; j<40 ; j++ )
			printf("%c",lines[j][i]);
		printf("\n");
	}
}
