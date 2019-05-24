#include  "sys/types.h"
#include  "sys/stat.h"

rmdir(f)
char *f;
{
	int status, i;
        struct stat st;

	if (-1 == stat(f, &st))
	        return(-1);
        while((i=fork()) == -1)
		sleep(3);
	if(i) {
		wait(&status);
	        if (stat(f, &st) == -1)
		      return(0);
	        return(-1);

	}
	execl("/bin/rmdir", "rmdir", f, 0);
	exit(1);
}
