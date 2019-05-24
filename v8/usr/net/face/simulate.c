#include "face.h"
#include <sys/param.h>	/* for NOFILE */
typedef struct Fptr{
	Inode	*inode;
	long	addr;
	int	fd;
}Fptr;
Fptr *fptr[NOFILE];
/*
 *	Simulate the system to the remote file system.
 *	There are essentially two parts,  the first using
 *	symbolic names and the second using fd's.
 */
simtyperror(s)
	char *s;
{
	error("unknown type in", s);
}

/*
 *	First the file name dependent commands
 */
extern int
simopen(name, mode)
	char *name;
	int mode;
{
	register Inode *ip=namei(name, &root);
	if(mode!=0){
		errno=EPERM;
		return -1;
	}
	if(ip==0)
		return -1;
	errno=0;
	switch(ip->type){
	case DIR:
	case REG:
		break;
	default:
		simtyperror("open");
	}
	return fptropen(ip);
}

extern int
simstat(name, bp)
	char *name;
	struct stat *bp;
{
	register Inode *ip=namei(name, &root);
	if(ip==0)
		return -1;
	errno=0;
	switch(ip->type){
	case DIR:
		*bp=ip->idir->statb;
		break;
	case REG:
		if(stat(ip->ifile->name, bp)==-1)
			return -1;
		purifystatb(bp, ip);
		break;
	default:
		simtyperror("open");
	}
	return 0;
}

/*
 *	FD specific routines
 */
extern long
simlseek(fd, offset, whence)
	int fd;
	long offset;
	int whence;
{
	register Fptr *fp;
	if((fp=fptr[fd])==0){
		errno=EBADF;
		return -1;
	}
	errno=0;
	switch(fp->inode->type){
	case DIR:
		switch(whence){
		case 0:
			return fp->addr=offset;
		case 1:
			return fp->addr+=offset;
		case 2:
			return fp->addr=fp->inode->idir->statb.st_size+offset;
		default:
			return -1;
		}
	case REG:
		return lseek(fp->fd, offset, whence);
	default:
		simtyperror("lseek");
	}
}

extern int
simread(fd, bp, count)
	int fd;
	char *bp;
	int count;
{
	register Fptr *fp;
	register len;
	if(count<0){
		errno=EIO;
		return -1;
	}
	if((fp=fptr[fd])==0){
		errno=EBADF;
		return -1;
	}
	errno=0;
	switch(fp->inode->type){
	case DIR:
		if(fp->addr<0 || fp->addr>fp->inode->idir->statb.st_size){
			errno=EINVAL;
			return -1;
		}
		if(fp->addr+count>fp->inode->idir->statb.st_size)
			count=fp->inode->idir->statb.st_size-fp->addr;
		bcopy(bp, ((char *)fp->inode->idir->entry)+fp->addr, count);
		fp->addr+=count;
		return count;
	case REG:
		return read(fp->fd, bp, count);
	default:
		simtyperror("read");
	}
}

extern int
simclose(fd)
	int fd;
{
	register Fptr *fp;
	if((fp=fptr[fd])==0){
		errno=EBADF;
		return -1;
	}
	errno=0;
	switch(fp->inode->type){
	case DIR:
		break;
	case REG:
		close(fp->fd);
		break;
	default:
		simtyperror("close");
	}
	free((char *)fp);
	fptr[fd]=0;
	return 0;
}

/*
 * Support routines
 */
int
newfd(){
	register fd;
	for(fd=3; fptr[fd]; )	/* 3 ==> skip stdin, stdout, stderr (??) */
		if(++fd>=NOFILE)
			error("too many open files", (char *)0);
	fptr[fd]=(Fptr *)emalloc(sizeof(Fptr));
	fptr[fd]->addr=0;
	return fd;
}

int
fptropen(ip)
	Inode *ip;
{
	register fd=newfd();
	register Fptr *fp=fptr[fd];
	fp->inode=ip;
	switch(ip->type){
	case DIR:
		break;
	case REG:
		fp->fd=open(ip->ifile->name, 0);
		if(fp->fd==-1){
			fprintf(stderr, "can't find file %s\n", ip->ifile->name);
			free((char *)fp);
			fptr[fd]=0;
			fd=-1;
		}
		break;
	default:
		simtyperror("fptropen");
	}
	return fd;
}
purifystatb(bp, ip)
	register struct stat *bp;
	register Inode *ip;
{
	bp->st_ino=ip->inumber;
	bp->st_mode&=~0333;
	bp->st_nlink=1;
	bp->st_uid=0;
	bp->st_gid=0;
}
bcopy(a, b, n)
	register char *a, *b;
	register n;
{
	while(n--)
		*a++=*b++;
}
