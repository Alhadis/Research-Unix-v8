#include "file.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

struct stat statbuf;
char	*mktemp(), *malloc();
long	lseek();
char	tempname[];
int	tempfile;

static	seek();
static	newblock();
static	relblock();
static	clearblocks();

#define	MAXFREE 512
static	nfree=0;	/* number of known free blocks */
static	next=0;		/* next available block in file */
static	short freelist[MAXFREE];
/*
 * Fill in block b with information for fresh block
 */
static
newblock(b, l)
	register Block *b;
{
	if(nfree==0)	/* no empty blocks to be reclaimed */
		b->bnum=next++;
	else
		b->bnum=freelist[--nfree];
	b->nbytes=l;
}
/*
 * Return block b to the free pool
 */
static
relblock(b)
	register Block *b;
{
if(b->bnum==0) abort();
	if(nfree<MAXFREE)
		freelist[nfree++]=b->bnum;
}
static
clearblocks(f)
	register File *f;
{
	register Block *b;
	/*
	 * Always leave the first block alive, and delete
	 * in reverse order so they reallocate in forward order
	 */
	for(b= &f->block[f->nblocks-1]; b>f->block; --b)
		relblock(b);
	f->nblocks=min(1, f->nblocks);
}
File *
Fcreat(f, s)
	register File *f;
	String *s;
{
	f->str=newstring();
	dupstring(s, f->name=newstring());
	newblock(f->block, 0);
	f->nblocks=1;
	f->curblock=0;
	f->nsel=0;
	f->origin=0;
	f->date=0;
	Fread(f, 0L, f->name->s, TRUE, 0);
	f->changed=FALSE;
	Fgetblock(f, 0);
	return f;
}
Fread(f, posn, s, setdate, r)
	register File *f;
	long posn;
	char *s;
{
	register n, nbytes=0;
	char buf[BLOCKSIZE];
	register Block *b, *here;

	if(r==0){
		if(s[0]==0)
			return;
		if((r=open(s, 0)) == -1){
			mesg("can't open", s);
			return;
		}
		if(setdate){
			fstat(r, &statbuf);
			f->date=statbuf.st_mtime;
		}
	}
	splitblock(f, Fseek(f, posn));
	for(here=f->curblock; (n=read(r, buf, BLOCKSIZE/2))>0; ){
		nbytes+=n;
		if(f->nblocks>=NBLOCK-1){
			close(r);
			toolarge(f);
		}
		for(b= &f->block[f->nblocks-1]; b>here; --b)
			b[1]=b[0];
		newblock(++here, n);
		f->nblocks++;
		seek(here->bnum);
		Write(tempname, tempfile, buf, BLOCKSIZE);
	}
	f->curblock=0;
	close(r);
	return nbytes;
}

/*
 * Dump file f to named file; but if fd>0, it's an open file
 * and the file should be appended to it.
 */
Fwrite(f, fname, fd)
	register File *f;
	register String *fname;
	register fd;
{
	char buf[BLOCKSIZE];
	register Block *b;
	register newfile=fd==0;

	Fputblock(f);
	if(fd==0){
		if(stat(fname->s, &statbuf)!=0)
			dprintf("new file; ");
		else if(f->date==DUBIOUS){
			f->date=statbuf.st_mtime;
			error("file already exists", (char *)0);
		}
		if(samefile(fname, f->name) && statbuf.st_mtime>f->date){
			f->date=statbuf.st_mtime;
			error("unix file modified since last read/written", (char *)0);
		}
		if((fd=creat(fname->s, 0666)) == -1)
			error("can't create", fname->s);
	}
	for(b=f->block; b<&f->block[f->nblocks]; b++){
		seek(b->bnum);
		Read(tempname, tempfile, buf, b->nbytes);
		Write(fname->s, fd, buf, b->nbytes);
	}
	if(buf[b[-1].nbytes-1]!='\n')
		dprintf("last char not newline; ");
	if(samefile(fname, f->name)){
		fstat(fd, &statbuf);
		f->date=statbuf.st_mtime;
	}
	if(newfile)
		close(fd);
}
/*
 * Fwritepart: write out nbytes from f at posn to file descriptor fd
 */
Fwritepart(f, posn, nbytes, fd)
	register File *f;
	long posn;
	register fd;
{
	register n, nthisblock;
	while(nbytes>0){
		if((n=Fseek(f, posn+1)-1)<0)
			break;
		nthisblock=min(f->str->n-n, nbytes);
		if(write(fd, f->str->s+n, nthisblock)!=nthisblock)
			break;
		posn+=nthisblock;
		nbytes-=nthisblock;
	}
	return nbytes!=0;
}
samefile(f1, f2)
	String *f1, *f2;
{
	struct stat stat2;
	return stat(f1->s, &statbuf)==0 /* else file doesn't exist, no problem */
	    && stat(f2->s, &stat2)==0
            && statbuf.st_dev==stat2.st_dev
	    && statbuf.st_ino==stat2.st_ino ;	/* the same file, for sure */
}

Fclose(f)
	register File *f;
{
	clearblocks(f);
	if(f->str)
		remstring(f->str);
	if(f->name)
		remstring(f->name);
	f->str=f->name=0;
}
Freset(f)
	register File *f;
{
	clearblocks(f);
	zerostring(f->str);
	f->origin=0;
	f->nsel=0;
	f->selloc=0;
}


toolarge(f)
	register File *f;
{
	error("temp file too large for", f->name->s);
}

Read(s, fd, a, n)
	char *s;
	int fd;
	char *a;
	int n;
{
	if(read(fd, a, n) != n)
		ioerr("read", s);
}

Write(s, fd, a, n)
	char *s;
	int fd;
	char *a;
	int n;
{
	if(write(fd, a, n) != n)
		ioerr("write", s);
}

/*
 * Bring into memory block n of File f
 */
Fgetblock(f, n)
	register File *f;
	register n;
{
	register Block *b=f->block+n;
	if(b==f->curblock || n<0 || n>=f->nblocks)	/* last two are just safety */
		return;
	insure(f->str, b->nbytes);
	seek(b->bnum);
	Read(tempname, tempfile, f->str->s, b->nbytes);
	f->str->n=b->nbytes;
	f->curblock=b;
}

/*
 * do lseek to the start of the named block 
 */
static
seek(b)
	register b;
{
	if(lseek(tempfile, b*(long)BLOCKSIZE, 0) == -1L)
		ioerr("lseek", tempname);
}

/*
 * Insert a new block into f after block f->curblock
 */
static Block *
makeblock(f, n)
	register File *f;
{
	register Block *b;

	if(f->nblocks>=NBLOCK-1)
		toolarge(f);
	for(b= &f->block[f->nblocks-1]; b>f->curblock; --b)
		b[1]=b[0];
	b=f->curblock+1;
	newblock(b, n);
	f->nblocks++;
	seek(b->bnum);
	return b;
}

/*
 * Write f->curblock to disk.  This may require generating
 * more disk blocks.  f->curblock is left at the BEGINNING
 * of the original block, but the associated string may be shorter.
 */
Fputblock(f)
	register File *f;
{
	register nbytes=f->str->n;

	if(f->curblock==0)
		return;
	while(nbytes>=BLOCKSIZE)	/* the = isn't necessary */
		splitblock(f, nbytes-=BLOCKSIZE/2);
	f->curblock->nbytes=f->str->n;
	seek(f->curblock->bnum);
	Write(tempname, tempfile, f->str->s, f->str->n);
}
/*
 * Split f->curblock by moving bytes after position n into new block
 */
splitblock(f, n)
	register File *f;
{
	register nchop=f->str->n-n;

	if(nchop==0)
		return 0;	/* why do anything? */
	(void)makeblock(f, nchop);
	Write(tempname, tempfile, f->str->s+n, nchop);
	delstring(f->str, n, nchop);
	f->curblock->nbytes-=nchop;
	return 1;
}
/*
 * Seek to absolute location m in file.  Pick up the block
 * and return character number of that location in the block.
 */
Fseek(f, m)
	register File *f;
	register long m;
{
	register long i;
	register Block *b;

	for(i=0,b=f->block; b<&f->block[f->nblocks]; b++){
		if(i+b->nbytes >= m)
			goto out;
		i+=b->nbytes;
	}
	/* If here, m is after EOF */
	return -1;
    out:
	if(f->curblock && f->curblock != b)
		Fputblock(f);
	Fgetblock(f, b-f->block);
	return (short)(m-i);
}

/*
 * Insert the n-character string s at location m in file f
 */
Finstext(f, m, s, n)
	register File *f;
	register long m;
	register char *s;
	int n;
{
	insstring(f->str, Fseek(f, m), s, n);
	if((f->curblock->nbytes=f->str->n)>BLOCKSIZE)	/* write the block */
		Fputblock(f);
	if(n>0)
		modified(f);
}

/*
 * Delete n characters at absolute location m in file f.
 */
Fdeltext(f, m, n)
	register File *f;
	register long m;
	register long n;
{
	register nbytes, loc;
	register Block *b;
	if(n>0)
		modified(f);
	while(n > 0){
		/*
		 * If string is at start of block, want to be in that block
		 * not the previous (as we want for insert).
		 */
		loc=Fseek(f, m+1)-1;
		nbytes=min((long)n, (long)f->str->n-loc);
		if(nbytes<=0)	/* at EOF */
			break;
		delstring(f->str, loc, nbytes);
		if((f->curblock->nbytes-=nbytes)<=0 && f->nblocks>1){
			relblock(f->curblock);
			for(b=f->curblock; b<&f->block[f->nblocks]; b++)
				b[0]=b[1];
			f->nblocks--;
			f->curblock=0;
		}
		n-=nbytes;
	}
}

/*
 * Set String b to string in file f at m, n chars long
 */
Fsave(f, b, m, n)
	register File *f;
	register String *b;
	register long m;
	register n;
{
	register loc, nbytes;
	register char *p;

	zerostring(b);
	insure(b, n);
	while(n>0){
		loc=Fseek(f, m+1)-1;
		if(loc<0)
			break;
		nbytes=min((long)n, (long)f->str->n-loc);
		if(nbytes<=0)
			break;
		for(p=f->str->s+loc; p<f->str->s+loc+nbytes; p++)
			b->s[b->n++] = *p;
		n-=nbytes;
		m+=nbytes;
	}
	b->s[b->n]=0;
}

/*
 * Return posn of char after first newline after posn, where
 * posn is expressed as a fraction (range 0-YMAX) of the total file length
 */
Forigin(f, posn)
	register File *f;
	register long posn;
{
	register n;
	register long l;
	register char *p;
	posn=((l=length(f))*posn)/YMAX;
	if(posn==0)
		return 0;
	if(posn>=l)
		return l;
	for(;;){
		n=Fseek(f, posn+1)-1;
		if(n<0)
			return(l);
		for(p=f->str->s+n; n<f->str->n; n++, p++, posn++)
			if(*p=='\n')
				return posn+1;
	}
}

/*
 * Count newlines before character position
 */
long
Fcountnl(f, charno)
	register File *f;
	register long charno;
{
	register long posn=0;
	register char *p;
	register nl=1;
	register n;
	charno=min(length(f), charno);
	for(;;){
		if((n=Fseek(f, posn+1)-1)<0)
			return nl;
		for(p=f->str->s+n; n<f->str->n; n++, p++, posn++){
			if(posn >= charno)
				return nl;
			if(*p=='\n')
				nl++;
		}
	}
}
/*
 * Number of characters forward to after nlines'th \n after posn
 */
long
Fforwnl(f, posn, nlines)
	register File *f;
	register long posn;
	register nlines;
{
	register long nchars=0;
	register char *p;
	register long l=length(f);
	register n;

	if(nlines<=0)
		return 0;
	for(;;){
		if(posn>=l || (n=Fseek(f, posn+1)-1)<0)
			return nchars;
		for(p=f->str->s+n; n<f->str->n; n++, p++, posn++, nchars++)
			if(*p=='\n' && --nlines<=0)
				return nchars+1;
	}
}

/*
 * Number of characters backward to after nlines'th \n before posn
 */
long
Fbacknl(f, posn, nlines)
	register File *f;
	register long posn;
	register nlines;
{
	register long nchars=0;
	register char *p;
	register n;

	if(nlines<=0)
		return 0;
	for(;;){
		if(posn <= 0)
			return nchars;
		n=Fseek(f, posn);
		if(n == 0){	/* should never happen */
			--posn;
			continue;
		}
		for(p=f->str->s+n-1; n>0; --n, --p, --posn, nchars++)
			if(*p=='\n' && --nlines<=0)
				return nchars;
	}
}

min(a, b)
	register long a, b;
{
	if(a<b)
		return a;
	return b;
}

long
length(f)
	register File *f;
{
	register Block *b=f->block;
	register long n=0;
	while(b<&f->block[f->nblocks])
		n+=b++->nbytes;
	return n;
}
