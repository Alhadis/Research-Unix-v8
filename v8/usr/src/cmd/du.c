#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#define EQ(x,y)	(strcmp(x,y)==0)
#define ML	1000

struct stat Statb;
char	path[BUFSIZE], name[BUFSIZE];
int	Aflag = 0,
	Sflag = 0,
	Noarg = 0;
struct {
	int	dev,
		ino;
} ml[ML];
long	descend();
char	*rindex();
char	*strcpy();

main(argc, argv)
char **argv;
{
	register	i = 1;
	long	blocks = 0;
	register char	*np;

	if (argc>1) {
		if(EQ(argv[i], "-s")) {
			++i;
			++Sflag;
		} else if(EQ(argv[i], "-a")) {
			++i;
			++Aflag;
		}
	}
	if(i == argc)
		++Noarg;

	do {
		strcpy(path, Noarg? ".": argv[i]);
		strcpy(name, path);
		if(np = rindex(name, '/')) {
			*np++ = '\0';
			if(chdir(*name? name: "/") == -1) {
				fprintf(stderr, "cannot chdir()\n");
				exit(1);
			}
		} else
			np = path;
		blocks = descend(path, *np? np: ".", 0);
		if(Sflag)
			printf("%ld\t%s\n", blocks, path);
	} while(++i < argc);

	exit(0);
}

long
descend(np, fname, depth)
char *np, *fname;
{
	int dir = 0, /* open directory */
		offset,
		dsize,
		entries,
		dirsize;

	struct direct dentry[BUFSIZE/sizeof(struct direct)];
	register  struct direct *dp;
	register char *c1, *c2;
	int i;
	char *endofname;
	long blocks = 0;

	if(lstat(fname,&Statb)<0) {
		perror(name);
		fprintf(stderr, "--bad status < %s %s >\n", fname, name);
		return 0L;
	}
	if(Statb.st_nlink > 1 && (Statb.st_mode&S_IFMT)!=S_IFDIR) {
		static linked = 0;

		for(i = 0; i <= linked; ++i) {
			if(ml[i].ino==Statb.st_ino && ml[i].dev==Statb.st_dev)
				return 0;
		}
		if (linked < ML) {
			ml[linked].dev = Statb.st_dev;
			ml[linked].ino = Statb.st_ino;
			++linked;
		}
	}
	blocks = ((Statb.st_size+BSIZE(Statb.st_dev)-1)>>BSHIFT(Statb.st_dev)) *
		 (BSIZE(Statb.st_dev)/BSIZE(0));

	if((Statb.st_mode&S_IFMT)!=S_IFDIR) {
		if(Aflag)
			printf("%ld\t%s\n", blocks, np);
		return(blocks);
	}

	if(Aflag)
		printf("%ld\t%s/\n", blocks, np);
	for(c1 = np; *c1; ++c1);
	if(*(c1-1) == '/')
		--c1;
	endofname = c1;
	dirsize = Statb.st_size;
	if(chdir(fname) == -1)
		return 0;
	for(offset=0; offset < dirsize; offset += BUFSIZE) { /* each block */
		dsize = BUFSIZE<(dirsize-offset)? BUFSIZE: (dirsize-offset);
		if(!dir) {
			if((dir=open(".",0))<0) {
				fprintf(stderr, "--cannot open < %s >\n",
					np);
				goto ret;
			}
			if(offset) lseek(dir, (long)offset, 0);
			if(read(dir, (char *)dentry, dsize)<0) {
				fprintf(stderr, "--cannot read < %s >\n",
					np);
				goto ret;
			}
			if(dir > 15) {
				close(dir);
				dir = 0;
			}
		} else 
			if(read(dir, (char *)dentry, dsize)<0) {
				fprintf(stderr, "--cannot read < %s >\n", np);
				goto ret;
			}
		entries = dsize / sizeof(struct direct);
		for(dp=dentry; entries; --entries, ++dp) {
			/* each directory entry */
			if(dp->d_ino==0 || EQ(dp->d_name, ".") || EQ(dp->d_name, ".."))
				continue;
			c1 = endofname;
			*c1++ = '/';
			c2 = dp->d_name;
			for(i=0; i<DIRSIZ; ++i)
				if(*c2)
					*c1++ = *c2++;
				else
					break;
			*c1 = '\0';
			if(c1 == endofname) /* ?? */
				return 0L;
			blocks += descend(np, endofname+1, depth+1);
		}
	}
	*endofname = '\0';
	if(!Sflag)
		printf("%ld\t%s\n", blocks, np);
ret:
	if(dir)
		close(dir);
	if(chdir("..") == -1) {
		*endofname = '\0';
		fprintf(stderr, "Bad directory <%s>\n", np);
		while(*--endofname != '/');
		*endofname = '\0';
		if(chdir(np) == -1)
			exit(1);
	}
	return(blocks);
}
