#include  "sys/types.h"
#include  "sys/stat.h"

mkdir(f, mode)
char *f;
{
	int status, i, mode;
        struct stat st;

        if (!stat(f, &st))
	        return(-1);
        while((i=fork()) == -1)
		sleep(3);
	if(i) {
		wait(&status);
		return(stat(f, &st));
	}
	execl("/bin/mkdir", "mkdir", f, 0);
	exit(1);
}
