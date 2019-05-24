#include "stdio.h"
#include "fserv.h"
#include "sys/neta.h"

#define FACEUID 1
#define FACEGID 1

hostuid(s)
struct stat *s;
{
	return(FACEUID);
}

hostgid(s)
struct stat *s;
{
	return(FACEGID);
}

myuid(u, g)
{
	return(FACEUID);
}

mygid(u, g)
{
	return(FACEGID);
}

noaccess(x, sbuf, how)
struct senda *x;
struct stat *sbuf;
{
	return(0);
}
