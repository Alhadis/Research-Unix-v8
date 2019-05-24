#include <sys/inet/in.h>
#include <stdio.h>
#include "config.h"

static char *files[] = {
	HOSTS,
	NETWORKS,
};
#define NFILES (sizeof(files) / sizeof(files[0]))

/* imported */
extern char *in_getw();

char *
in_host(addr)
in_addr addr;
{
	char buf[512], *p;
	static char b[32];
	int x, i;
	FILE *fp;
	unsigned char *xp;

	xp = (unsigned char *) &addr;
	for(i = 0 ; i < NFILES; i++){
		if((fp = fopen(files[i], "r")) == 0){
			perror(files[i]);
			continue;
		}
	
		while(fgets(buf, sizeof(buf), fp)){
			if(buf[0] == '\n' || buf[0] == '#')
				continue;
			if((p = in_getw(buf, b)) == 0)
				continue;
			x = in_address(b);
			if(x == 0)
				continue;
			if((p = in_getw(p, b)) == 0)
				continue;
			if(x == addr){
				fclose(fp);
				return(b);
			}
		}
		fclose(fp);
	}
	sprintf(b, "%d.%d.%d.%d", xp[3], xp[2], xp[1], xp[0]);
	return(b);
}
