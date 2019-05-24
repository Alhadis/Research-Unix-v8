#include "stdio.h"
#include "a.out.h"
#include "mld.h"
char *cfd;
int outtype = 0407;		/* default type of output file to produce */
list syslibs, usrlibs;		/* lists of library directories.
					usr searched first */
char *outfile = "a.out";	/* default output file */
extern int cntundef;		/* running count of undefined externals */
char flg[128];
extern int errcnt, dataorg, bssorg;
char *me;
extern char *malloc();
extern file *tofile();

main(argc, argv) char **argv;
{	int i;
	me = argv[0];
	syslibs = initlist;
	usrlibs = initlist;
	append(&syslibs, "/usr/lib");
	append(&syslibs, "/usr/blit/lib");

	doarg(++argv, --argc);	/* decide which files to load */
	if(dataorg || bssorg) {
		flg['r'] = 0;
		flg['R'] = 0;
		warn("(warning) data origin prohibits saving reloc bits\n");
	}
	if(errcnt)
		exit(1);
	fakefile();	/* to hold ld defined symbols */
	globaddr();		/* assign addresses to globals */
	newstab();		/* output symbol table */
	reloc();		/* relocate everything */
	output();
	exit(0);
}

fakefile()
{	char *s;
	file *f;
	struct exec *u;
	s = malloc(sizeof(struct exec) + 6 * sizeof(struct nlist));
	/* space for end, edata, etext, data, and two more */
	u = (struct exec *)s;
	u->a_magic = 0407;
	u->a_text = u->a_data = u->a_bss = u->a_entry = u->a_unused = 0;
	u->a_syms = u->a_flag = 0;
	f = tofile("(ldr)", s);
	append(mustuse, (char *)f);
}
