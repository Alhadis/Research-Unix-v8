#include "face.h"
ushort	inumber=3;
#define	NDIRCHUNK	32
Inode *
newi(type)
{
	register Inode *ip=(Inode *)permalloc(sizeof(Inode));
	ip->inumber=inumber++;
	ip->type=type;
	return ip;
}
Inode *
newd(parent)
	Inode *parent;
{
	register Inode *ip=newi(DIR);
	ip->idir=(Dir *)permalloc(sizeof(Dir));
	initd(ip->idir, ip, parent);
	return ip;
}
Inode *
newf(s)
	char *s;
{
	register Inode *ip=newi(REG);
	ip->ifile=(File *)permalloc(sizeof(File));
	ip->ifile->name=dupstr(s);
	return ip;
}
attach(s, i, d)
	register char *s;
	Inode *i;
	Dir *d;
{
	if(tlookup(s, d->trie)){
		fprintf(stderr, "duplicate directory entry %s (ignored)\n", s);
		return;
	}
	if(d->nentry>=d->nalloc)
		d->entry=(struct direct *)erealloc((char *)d->entry,
			(d->nalloc+=NDIRCHUNK)*sizeof(struct direct));
	tinsert(s, (Trie *)i, d->trie);
	directset(&d->entry[d->nentry], i, s);
	lengthd(d, 1);
}
Inode *
namei(path, ip)
	register char *path;
	register Inode *ip;
{
	char buf[128];	/* safety first */
	register n;
	register char *p;
	for(;;){
		while(*path=='/')
			path++;
		if(*path==0)
			break;
		if(ip->type!=DIR){
			errno=ENOTDIR;
			return 0;
		}
		if(p=strchr(path, '/'))	/* assignment = */
			n=p-path;
		else
			n=strlen(path);
		strncpy(buf, path, n);
		buf[n<DIRSIZ? n : DIRSIZ]=0;
		path+=n;
		ip=(Inode *)tlookup(buf, ip->idir->trie);
	}
	if(ip==0)
		errno=ENOENT;
	return ip;
}
initd(d, self, parent)
	register Dir *d;
	Inode *self, *parent;
{
	tinsert("..", (Trie *)parent, d->trie=tcreate(".", (Trie *)self));
	d->entry=(struct direct *)emalloc(NDIRCHUNK*sizeof(struct direct));
	d->nalloc=NDIRCHUNK;
	d->nentry=0;
	d->statb.st_size=0;
	lengthd(d, 2);
	directset(&d->entry[0], self, ".");
	directset(&d->entry[1], parent, "..");
	d->statb.st_dev=0;
	d->statb.st_ino=self->inumber;
	d->statb.st_mode=S_IFDIR|0555;
	d->statb.st_uid=d->statb.st_gid=0;
	d->statb.st_nlink=2;
	d->statb.st_atime=d->statb.st_mtime=d->statb.st_ctime=time((long *)0);
}
directset(d, i, s)
	register struct direct *d;
	struct Inode *i;
	char *s;
{
	static struct direct zerodir;
	*d=zerodir;	/* clear out file name */
	d->d_ino=i->inumber;
	strncpy(d->d_name, s, DIRSIZ);
}
lengthd(d, n)
	register Dir *d;
{
	d->nentry+=n;
	d->statb.st_size+=n*sizeof(struct direct);
}
