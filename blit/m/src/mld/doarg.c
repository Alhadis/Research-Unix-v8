#include "a.out.h"
#include "ar.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "ctype.h"
#include "mld.h"

char flg[128];
extern char *malloc(), *cat4();
char *outfile;
int textorg, dataorg, bssorg;
char textset, dataset, bssset;
struct exec outhdr = {0407};
extern list usrlibs, syslibs;

doarg(a, n)
char **a;
{	int i, fd;
	char *p;
	struct exec *u;
	struct stat stb;
	for(i = 0; i < n; i++) {
		fd = open(a[i], 0);
		if(fd < 0) {
			i += options(a + i, n - i);
			i--;
			continue;
		}
		if(fstat(fd, &stb) < 0) {
			perror("stat");
			exit(1);
		}
		p = malloc(stb.st_size);
		if(p == 0)
			fatal("no space for file %s\n", a[i]);
		read(fd, p, stb.st_size);
		close(fd);
		u = (struct exec *)p;
		switch(u->a_magic) {
		case 0406:
		case 0407:
			doto(a[i], p);
			continue;
		case 0410:
		case 0411:
		case 0413:
			fatal("type 0%0 file %s on input\n", u->a_magic, a[i]);
			break;
		}
		if(strncmp(ARMAG, p, SARMAG) == 0) {
			doarch(a[i], p, stb.st_size);
			continue;
		}
		cmdfile(a[i], p, stb.st_size);
	}
}

cmdfile(nm, s, n)
char *s, *nm;
{	int i, cnt;
	char **p, **q;
	cnt = 0;
	for(i = 0; i < n; i++) {
		if(!isspace(s[i]) && !isprint(s[i]))
			fatal("file %s incomprehensible\n", nm);
		if(i == 0 && !isspace(s[i])
			|| i > 0 && isspace(s[i-1]) && !isspace(s[i]))
				cnt++;
	}
	if(cnt == 0)
		return;
	q = p = (char **)malloc(cnt * sizeof(char *));
	if(!isspace(s[0]))
		*p++ = s;
	for(i = 1; i < n; i++) {
		if(isspace(s[i]))
			s[i] = 0;
		else if(!s[i-1])
			*p++ = s + i;
	}
	if(cnt != p - q)
		fatal("cmdfile internal error\n");
	verbose("cmd file %s\n", nm);
	doarg(q, cnt);
}

options(a, n)
char **a;
{	char *p;
	int cnt = 0;
	p = a[0];
	if(*p++ != '-') {
		warn("can't open %s\n", p-1);
		return(1);
	}
	for(; *p; p++)
	switch(*p) {
	default:
		error("unknown option %c\n", *p);
		return(++cnt);
	case 'o':
		if(++cnt >= n)
			error("no -o file given\n");
		outfile = a[cnt];
		verbose("-o %s\n", outfile);
		break;
	case 'b':
		if(++cnt >= n)
			error("no -b origin given\n");
		textorg = atoi(a[cnt]);
		textset = 1;
		verbose("textorg %d\n", textorg);
		break;
	case 'B':
		if(++cnt >= n)
			error("no -B origin given\n");
		bssorg = atoi(a[cnt]);
		bssset = 1;
		verbose("bssorg %d\n", bssorg);
		break;
	case 'D':
		if(++cnt >= n)
			error("no -D (dataorg) origin given\n");
		dataorg = atoi(a[cnt]);
		dataset = 1;
		verbose("dataorg %d\n", dataorg);
		break;
	case 'd':	/* gen bss from comm */
		flg['d']++;
		verbose("-d: change common to bss\n");
		break;
	case 'u':	/* make name undefined */
		if(++cnt >= n)
			error("no name after -u\n");
		toundef(a[cnt]);
		verbose("-u makes %s undefined\n", a[cnt]);
		break;
	case 'l':
		dolib(p+1);
		return(++cnt);
	case 'M':
		verbose("0406 file\n");
		outhdr.a_magic = 0406;
		break;
	case 'm':
		flg['m']++;
		verbose("-m\n");
		break;
	case 'r':
		flg['r']++;
		verbose("-r\n");
		break;
	case 'v':
		flg['v']++;
		break;
	case 'R':	/* -r, but error if undefs left */
		flg['R']++;
		flg['r']++;
		flg['d']++;
		verbose("-R\n");
		break;
	case 's':
		flg['s']++;
		verbose("-s\n");
		break;
	}
	return(++cnt);
}

dolib(s)
char *s;
{	int i;
	for(i = 0; i < usrlibs.lnext; i++)
		ldlib(s, usrlibs.ldata[i]);
	for(i = 0; i < syslibs.lnext; i++)
		ldlib(s, syslibs.ldata[i]);
}

ldlib(s, lib)
char *s, *lib;
{	char *x, *y;
	struct stat stb;
	int fd;
	x = cat4(lib, "/lib", s, ".a");
	fd = open(x, 0);
	if(fd < 0)
		return;
	fstat(fd, &stb);
	y = malloc(stb.st_size);
	read(fd, y, stb.st_size);
	close(fd);
	doarch(x, y, stb.st_size);
}
