#include <sys/inet/in.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"

static char *files[] = {
	HOSTS,
	NETWORKS,
};
#define NFILES (sizeof(files) / sizeof(files[0]))

extern char *in_getw();

/* translate a host name into an address */
in_addr
in_address(host)
char *host;
{
	FILE *fp;
	char buf[512], b[512], *p;
	int addr, i;

	if(isdigit(host[0]))
		return(in_aton(host));
	for(i = 0; i < NFILES; i++){
		if((fp = fopen(files[i], "r")) == 0){
			perror(files[i]);
			continue;
		}
		while(fgets(buf, sizeof(buf), fp)){
			if(buf[0] == '\n' || buf[0] == '#')
				continue;
			if((p = in_getw(buf, b)) == 0)
				continue;
			addr = in_aton(b);
			while(p = in_getw(p, b)){
				if(strcmp(b, host) == 0){
					fclose(fp);
					return(addr);
				}
			}
		}
		fclose(fp);
	}
	return(0);
}
