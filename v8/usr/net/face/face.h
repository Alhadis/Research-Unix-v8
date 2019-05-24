#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <errno.h>
#include "trie.h"
typedef unsigned short	ushort;
typedef struct Dir{
	Trie		*trie;		/* trie of contents */
	struct direct	*entry;		/* contents in file system form */
	struct stat	statb;		/* buffer for fstat(), stat() sys calls */
	ushort		nentry;		/* high water mark in entries */
	ushort		nalloc;		/* number of entries allocated */
}Dir;
typedef struct File{
	char		*name;		/* name of associated Unix file */
}File;
typedef struct Inode{			/* analogous to i-node */
	ushort		type;		/* DIR or REG */
	ushort		inumber;
	union{
	  Dir		*Uidir;
	  File		*Uifile;
	}u;
}Inode;
#define	idir	u.Uidir
#define	ifile	u.Uifile
/*
 * Fnode types
 */
#define	DIR	S_IFDIR			/* why not? */
#define	REG	S_IFREG
extern int	errno;
extern Inode	root;
extern char	*malloc();
extern char	*emalloc();
extern char	*permalloc();
extern char	*realloc();
extern char	*erealloc();
extern char	*strcpy();
extern char	*strncpy();
extern char	*dupstr();
extern char	*strchr();
extern char	*gets();
extern long	lseek();
extern long	time();
extern FILE	*efopen();
extern Inode	*namei();
extern Inode	*newd();
extern Inode	*newf();
