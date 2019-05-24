
/*
 *	cp source dest
 *
 *	     or
 *
 *	cp source1 ... sourcen dest
 *
 *	In the first case, dest must not be a directory (else we have a
 *	degenerate version of the second case). In the second case,
 *	dest must be a directory; the resulting objects will be
 *	named dest/source1 ... dest/sourcen.
 *
 *	If any source is a directory, the tree structure under it
 *	will be (recursively) copied; this process will attempt to
 *	preserve any links that exist within the hierarchy.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

#define BLKSIZE BUFSIZ

/* Maximum file tree nesting before we start closing and re-opening */
#define MAXLEVEL 8

/* Non-zero if stat buffer b refers to a directory, zero otherwise */
#define isdir(b) (((b).st_mode & S_IFMT) == S_IFDIR)

char *malloc(), *realloc(), *strcpy(), *strcat();
int iamsu;

/* Final component of the path name by which we were invoked */
char *pgmname;

main (argc, argv)
	int argc;
	char **argv;
{
	int retcode;
	char *dest, *destdir, *p, *q;
	struct stat *argbufs, destbuf, statbuf, rootbuf;
	int i;

	umask(0);
	pgmname = argv[0];

	/*
	 *	Start validity checking
	 */

	/* Check for too few arguments given */
	if (argc < 3) {
		fprintf (stderr, "usage:  %s f1 f2  or  %s f1 ... fn d\n",
				pgmname, pgmname);
		return 1;
	}

	/*
	 *	If the last argument is a non-directory, we require
	 *	argc == 3 (in the case of: cp f1 f2)
	 */
	dest = argv[argc-1];
	destbuf.st_mode = 0;	/* Force isdir(destbuf) false */
	if ((stat (dest, &destbuf) < 0 || !isdir(destbuf)) && argc != 3) {
		scream ("%s not a directory", dest);
		return 1;
	}

	/*
	 *	Allocate storage for a stat buffer for each argument
	 *	but the last. This will be used to check for trying to
	 *	copy something into a subdirectory of itself.
	 */
	argbufs = (struct stat *)
		malloc ((unsigned) (sizeof (struct stat) * (argc-2)));
	if (argbufs == NULL) {
		scream ("insufficient storage", "");
		return 1;
	}
	iamsu = (getuid() == 0);

	for (i = 0; i < argc-2; i++) {
		if (stat (argv[i+1], &argbufs[i]) < 0) {
			scream ("cannot access %s", argv[i+1]);
			return 1;
		}
	}
	
	/*
	 *	Now run back from the destination to the root directory,
	 *	checking along the way for any matches with anything in
	 *	the argbufs array. If we find any, we tried a forbidden
	 *	operation. If the destination is not already a directory,
	 *	we must start from its parent. Note that even in this case,
	 *	we can't elide the test, because otherwise we wouldn't get
	 *
	 *		cp . x
	 *
	 *	This code also handles pathological but legal destinations
	 *	properly; in particular note that the null string is a
	 *	valid name for the current directory. Sigh.
	 *
	 *	First figure out the name of the directory to check.
	 */
	destdir = malloc ((unsigned) (strlen (dest) + 2));
	if (destdir == NULL) {
		scream ("insufficient storage", "");
		return 1;
	}
	strcpy (destdir, dest);
	if (!isdir (destbuf)) {
		p = q = destdir;
		while (*q) {
			if (*q++ == '/')
				p = q - 1;
		}
		if (p == destdir)
			strcpy (destdir, ".");
		else
			*p = '\0';
	}

	/* Now stat the root directory for later use */
	if (stat ("/", &rootbuf) < 0) {
		scream ("cannot access root directory", "");
		return 1;
	}

	/* Now run back from destdir to the root, checking against argbufs */
	do {
		if (stat (destdir, &statbuf) < 0) {
			scream ("cannot stat %s", destdir);
			return 1;
		}
		for (i = 0; i < argc-2; i++)
			if (argbufs[i].st_dev == statbuf.st_dev
			 && argbufs[i].st_ino == statbuf.st_ino) {
				fprintf (stderr, "%s: %s contains %s\n",
					pgmname, argv[i+1], dest);
				return 1;
			}
		destdir = realloc (destdir, (unsigned) (strlen(destdir) + 4));
		if (destdir == NULL) {
			scream ("insufficient storage", "");
			return 1;
		}
		strcat (destdir, "/..");
	} while (statbuf.st_dev != rootbuf.st_dev
	      || statbuf.st_ino != rootbuf.st_ino);
	free (destdir);

	/*
	 *	End of validity checking
	 */

	if (isdir (destbuf)) {
		retcode = 0;
		for (i=1; i<argc-1; i++) {

			char *p, *q;
			char *xdest;

			p = q = argv[i];
			while (*p)
				if (*p++ == '/')
					q = p;
			xdest = malloc ((unsigned)
				(strlen(dest) + strlen (q) + 2));
			if (xdest != NULL) {
				strcpy (xdest, dest);
				strcat (xdest, "/");
				strcat (xdest, q);
				retcode |= copy (argv[i], &argbufs[i-1], xdest);
				free (xdest);
			} else {
				scream ("no storage", "");
				retcode = 1;
			}
		}
	} else
		retcode = copy (argv[1], &argbufs[0], dest);

	return retcode;
}

/*
 *	Copy a file or directory. The source file name is in "source",
 *	and the destination file name is in "dest". "srcbuf" points to
 *	a stat buffer for the source.
 */
int
copy (source, srcbuf, dest)
	char *source, *dest;
	struct stat *srcbuf;
{
	int inf, outf, n;
	char buffer[BLKSIZE];
	struct stat newbuf, destbuf;
	static int level = 0;
	static short firstyell = 1;
	struct {
		long actime, modtime;
	} utimbuf;
	long time();
	static struct linknode {
		dev_t ln_dev;
		ino_t ln_ino;
		struct linknode *ln_next;
		char *ln_name;		/* variable */
	} *ln_head = NULL, *lnp;

	if (isdir (*srcbuf)) {
		
		char *srcname, *destname;
		FILE *dirfile;
		struct direct dir;
		long dirloc;
		int retcode = 0;

		/*
		 *	Copy from a directory. This will have to copy the
		 *	tree structure recursively. Thus, we will create
		 *	a new directory of an appropriate name, then
		 *	copy everything in source to elements of dest.
		 */
		if (stat (dest, &destbuf) < 0) {
			if (mkdir (dest)) {
				scream ("cannot create directory %s", dest);
				return 1;
			}
		} else {
			scream ("%s already exists", dest);
			return 1;
		}

		if ((dirfile = fopen (source, "r")) == NULL) {
			scream ("cannot open %s", source);
			return 1;
		}

		srcname = malloc ((unsigned) (strlen(source) + DIRSIZ + 2));
		destname = malloc ((unsigned) (strlen(dest) + DIRSIZ + 2));
		if (srcname == NULL || destname == NULL) {
			scream ("insufficient storage", "");
			fclose(dirfile);
			return 1;
		}

		while (fread ((char *) & dir, sizeof (struct direct),
			1, dirfile) == 1)
			if (dir.d_ino != 0 &&
			    strncmp (dir.d_name, ".", DIRSIZ) &&
			    strncmp (dir.d_name, "..", DIRSIZ)) {
				strcpy (srcname, source);
				strcat (srcname, "/");
				strncat (srcname, dir.d_name, DIRSIZ);
				strcpy (destname, dest);
				strcat (destname, "/");
				strncat (destname, dir.d_name, DIRSIZ);

				if (level > MAXLEVEL) {
					dirloc = ftell (dirfile);
					fclose (dirfile);
					dirfile = NULL;
				}

				if (lstat (srcname, &newbuf) < 0) {
					scream ("cannot stat %s", srcname);
					if(dirfile != NULL)
						fclose(dirfile);
					return 1;
				}

				level++;
				retcode |= copy (srcname, &newbuf, destname);
				level--;

				if (level > MAXLEVEL) {
					if ((dirfile = fopen (source, "r"))
					    == NULL) {
						scream ("cannot reopen %s",
						    source);
						return 1;
					}
					if(fseek(dirfile, dirloc, 0) == EOF) {
						scream("can't seek on %s",
							source);
						fclose(dirfile);
						return(1);
					}
				}
		}

		free (srcname);
		free (destname);
		if(iamsu)
			chown(dest, srcbuf->st_uid, srcbuf->st_gid);

		utimbuf.actime = time ((long *) NULL);
		utimbuf.modtime = srcbuf->st_mtime;
		utime (dest, &utimbuf);

		if (chmod (dest, srcbuf->st_mode & 07777) < 0) {
			scream ("cannot chmod for %s", dest);
			retcode = 1;
		}
		fclose(dirfile);
		return retcode;
	}

	/*
	 *	Here when src is not a directory
	 */
	if (stat (dest, &destbuf) >= 0
	 && srcbuf->st_dev == destbuf.st_dev
	 && srcbuf->st_ino == destbuf.st_ino) {
		scream ("cannot copy %s to itself", source);
		return 1;
	}

	/*
	 *	Check if the source (which is known to be a file) has
	 *	more than one link. If not, all is easy. If so, we may
	 *	be able to create a link rather than copying the source.
	 */
	if (srcbuf->st_nlink > 1) {

		/* Hunt through the table of linked items for a match */
		lnp = ln_head;
		while (lnp != NULL &&
		   (lnp->ln_dev != srcbuf->st_dev
		   ||  lnp->ln_ino != srcbuf->st_ino)) {
			lnp = lnp->ln_next;
		}

		/* If match found, try a link */
		if (lnp != NULL)
			if (link (lnp->ln_name, dest)) {
				scream ("cannot create %s link", dest);
				return 1;
			} else
				return 0;

		/* No match found -- build a table entry */
		lnp = (struct linknode *) malloc (sizeof (struct linknode));

		if (lnp == NULL) {
			if (firstyell) {
				scream ("link table too large", "");
				firstyell = 0;
			}
		} else if ((lnp->ln_name = malloc ((unsigned) (strlen(dest)+1)))
			    == NULL) {
			if (firstyell) {
				scream ("link names too large", "");
				firstyell = 0;
			}
			free ((char *) lnp);
		} else {
			lnp->ln_dev = srcbuf->st_dev;
			lnp->ln_ino = srcbuf->st_ino;
			strcpy (lnp->ln_name, dest);
			lnp->ln_next = ln_head;
			ln_head = lnp;
		}
	}

	/* Now copy the file, whether or not there was a link */
	switch(srcbuf->st_mode & S_IFMT){
	case S_IFLNK:{
		int len;
		char buf[1024];
		len = readlink(source, buf, sizeof buf);
		if(len<=0){
			scream("can't read link for %s", source);
			return 1;
		}
		if(symlink(buf, dest)!=0){
			scream("can't write link for %s", dest);
			return 1;
		}
		break;
	}
	case S_IFCHR:
	case S_IFBLK:
		if(mknod(dest, srcbuf->st_mode, srcbuf->st_rdev)!=0){
			scream("can't mknod %s", dest);
			return 1;
		}
		break;
	default:
		if ((outf = creat (dest, srcbuf->st_mode & 07777)) < 0) {
			scream ("cannot create %s", dest);
			return 1;
		}
		if ((inf = open (source, 0)) < 0) {
			scream ("cannot open %s", source);
			close(outf);
			return 1;
		}
		while ((n = read (inf, buffer, BLKSIZE)) > 0) {
			if (write (outf, buffer, n) != n) {
				scream ("output error on %s", dest);
				close (inf);
				close (outf);
				return 1;
			}
		}
		if (n < 0) {
			scream ("input error on %s", source);
			close (inf);
			close (outf);
			return 1;
		}
		if (close (outf) < 0) {
			scream ("cannot close %s", dest);
			close (inf);
			return 1;
		}
		if (close (inf) < 0) {
			scream ("cannot close %s", source);
			return 1;
		}
		break;
	}
	utimbuf.actime = srcbuf->st_atime;
	utimbuf.modtime = srcbuf->st_mtime;
	utime (dest, &utimbuf);
	if(iamsu)
		chown(dest, srcbuf->st_uid, srcbuf->st_gid);
	return 0;
}

int
mkdir(d)
	char *d;
{
	int status, pid, w;
	/*w = open(".", 0);
	printf("%d\t%s\n", w, d);
	close(w);*/
	switch (pid=fork()) {
	case 0:
		execl ("/bin/mkdir", "mkdir", d, 0);
		/* No break */
	case -1:
		return 1;

	default:
		do w = wait (&status);
		while (w != pid && w > 0);
		if (w == pid)
			return status;
		return w;
	}
}

scream (s1, s2)
	char *s1, *s2;
{
	fprintf (stderr, "%s: ", pgmname);
	fprintf (stderr, s1, s2);
	putc ('\n', stderr);
}
