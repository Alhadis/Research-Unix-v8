#ifndef LIB_H
#define LIB_H
char 	*strcpy(char*, char*),
	*strcat(char*, char*),
	*strncpy(char*, char*, int),
	*strncat(char*, char*, int),
	*basename(char*),
	*slashname(char*),
	*ctime(long*);

long lseek(int,long,int);
int ReadOK(int, char*, int);
int WriteOK(int, char*, int);

int	ioctl(int,int ...),
	open(char*, int),
	sscanf(char*, char* ...),
	sprintf(char*, char* ...),
	strcmp(char*,char*),
	strncmp(char*,char*,int),
	fstat(int, struct stat*),
	alarm(unsigned),
	getpid(),
	close(int),
	system(char*),
	fork(),
	sleep(unsigned),
	chdir(char*),
	stat(char*,struct stat*),
	tdkexec(char*,char*),
	killpg(int,int);

char *Name(char*,int);
char *SysErr(char* = "");

long modified(int);

int	alldigits(char*);

char	*calloc(int,int);
char	*malloc(int);
#endif
