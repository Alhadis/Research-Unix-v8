/*	@(#)hashmake.c	1.1	*/
#include "hash.h"
#include <stdio.h>

main()
{
	char word[30];
	long hashval;
	long h;
	hashinit();
	while(gets(word)) {
		hashval = hash(word);
		if(hashval == (1<<HASHWIDTH)-1) {
			fprintf(stderr,"hashmake: rejecting %s\n",word);
			continue;
		}
		printf("%.*lo\n",(HASHWIDTH+2)/3,hashval);
	}
	return(0);
}
