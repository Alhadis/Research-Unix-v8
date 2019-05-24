#define SIDE 40
/*
**  Make a C declaration initialization from a SIDExSIDE font picture
**
*/
#include <stdio.h>
main(argc, argv)
int argc;
char *argv[];
{
        char line[SIDE][SIDE+10];
        int i,j,chr;
	int rot,blit;
        unsigned char chrs[SIDE][SIDE/8];

	rot = 0;
	blit = 0;
	if ( argc > 1 && strcmp(argv[1],"-r")==0 ) {
		rot = 1;
		argc--; argv++;
	}
	if ( argc > 1 && strcmp(argv[1],"-b")==0 ) {
		blit = 1;
		argc--; argv++;
	}
        if ( argc > 1 )
                freopen(argv[1],"r",stdin);
        for (i=0;i<SIDE;i++) {
		gets(line[i]);
		for ( j=strlen(line[i]) ; j < SIDE ; j++ )
			strcat(line[i]," ");
		for ( j=0 ; j<SIDE/8 ; j++ )
			chrs[i][j] = 0;
	}
	if ( rot ) {
		for ( i=SIDE-1 ; i>=0 ; i-- ) { /* across */
			for ( j=SIDE-1 ; j>=0 ; j-- ) { /* down */
				chrs[SIDE-i-1][SIDE/8-j/8-1] =
(chrs[SIDE-i-1][SIDE/8-j/8-1]<<1)|(line[j][i]!=' '?1:0);
			}
		}
		
	} else if ( blit ) {
		for ( i=0 ; i<SIDE ; i++ ) { /* across */
			for ( j=0 ; j<SIDE ; j++ ) { /* down */
			chrs[i][j/8] = (chrs[i][j/8]<<1)|(line[i][j]!=' '?1:0);
			}
		}
		
	} else {
		for ( i=0 ; i<SIDE ; i++ ) { /* across */
			for ( j=0 ; j<SIDE ; j++ ) { /* down */
			chrs[i][j/8] = (chrs[i][j/8]<<1)|(line[j][i]!=' '?1:0);
			}
		}
		
	}
	printf("\t{ ");
        for ( i=0;i<SIDE;i++ ) {
		for ( j=0 ; j<SIDE/8 ; j++ )
			printf("0%03o, ",chrs[
blit?
						i
:
						SIDE-1-i
							][j]&0xff);
		if ( i%2 == 1 && i<SIDE-1 )
			printf("\n\t  ");
		else
			printf(" ");
        }
	printf(" },\n");
}
