/*
**	print symbol tables for
**	object or archive files
**
**	mnm [-goprun] [name ...]
*/



#include	<ar.h>
#include	"a.out.h"
#include	<stdio.h>
#include	<ctype.h>
#define	BADMAG(x)	x!=A_MAGIC1 && magbuf.i!=A_MAGIC2  \
		&& x!=A_MAGIC3 && magbuf.i!=A_MAGIC4
#define	SELECT	arch_flg ? arp.ar_name : *argv
int	numsort_flg;
int	undef_flg;
int	revsort_flg = 1;
int	debug_flg;	/* print debugging information; off by default */
int	globl_flg;
int	nosort_flg;
int	arch_flg;
int	prep_flg;
struct	ar_hdr	arp;
long	arsize;
struct	exec	exp;
FILE	*fi;
long	off;
long	atol();
long	ftell();
char	*malloc();
char	*realloc();

main(argc, argv)
char **argv;
{
	int narg;
	int  compare();
	union {
		char	a[SARMAG+1];
		int	i;
	} magbuf;
	int didone;

	didone=0;

	if (--argc>0 && argv[1][0]=='-' && argv[1][1]!=0) {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'd':		/* print debugging tables */
			debug_flg++;
			break;

		case 'n':		/* sort numerically */
			numsort_flg++;
			continue;

		case 'g':		/* globl symbols only */
			globl_flg++;
			continue;

		case 'u':		/* undefined symbols only */
			undef_flg++;
			continue;

		case 'r':		/* sort in reverse order */
			revsort_flg = -1;
			continue;

		case 'p':		/* don't sort -- symbol table order */
			nosort_flg++;
			continue;

		case 'o':		/* prepend a name to each line */
			prep_flg++;
			continue;

		default:		/* oops */
			fprintf(stderr, "mnm: invalid argument -%c\n", *argv[0]);
			exit(1);
		}
		argc--;
	}
	if (argc == 0) {
		argc = 1;
		argv[1] = "a.out";
	}
	narg = argc;
	while(argc--) {
		fi = fopen(*++argv,"r");
		if (fi == NULL) {
			fprintf(stderr, "mnm: cannot open %s\n", *argv);
			continue;
		}
		off = SARMAG;
		fread((char *)&magbuf, 1, sizeof(magbuf), fi);	/* get magic no. */
		if (strncmp(magbuf.a, ARMAG, SARMAG)==0)
			arch_flg++;
		else if (BADMAG(magbuf.i)) {
			fprintf(stderr, "mnm: %s-- bad format\n", *argv);
			continue;
		}
		fseek(fi, 0L, 0);
		if (arch_flg) {
			nextel(fi);
			if (narg > 1)
				printf("\n%s:\n", *argv);
		}
		do {
			long o;
			register i, n, c;
			struct nlist *symp = NULL;
			struct nlist sym;

			fread((char *)&exp, 1, sizeof(struct exec), fi);
			if (BADMAG(exp.a_magic)) {		/* archive element not in  */
				continue;	/* proper format - skip it */
			}
			o = (long)exp.a_text + exp.a_data;
			if ((exp.a_flag & 01) == 0)
				o *= 2;
			fseek(fi, o, 1);
			n = exp.a_syms / sizeof(struct nlist);
			if (n == 0) {
				fprintf(stderr, "mnm: %s-- no name list\n", SELECT);
				continue;
			}
			symp = (struct nlist *)malloc(n * sizeof(struct nlist));
			if (symp == 0) {
				fprintf(stderr, "mnm: %s-- can't malloc\n", SELECT);
				continue;
			}
			i = 0;
			while (--n >= 0) {
				fread((char *)&sym, 1, sizeof(sym), fi);
				if (globl_flg && (sym.n_type&N_EXT)==0)
					continue;
				if (!debug_flg && sym.n_dtype)
					continue;
				switch (sym.n_type&N_TYPE) {

				case N_UNDF:
					c = 'u';
					if (sym.n_value)
						c = 'c';
					break;

				default:
				case N_ABS:
					c = 'a';
					break;

				case N_TEXT:
					c = 't';
					break;

				case N_DATA:
					c = 'd';
					break;

				case N_BSS:
					c = 'b';
					break;

				}
				didone=1;
				if (undef_flg && c!='u')
					continue;
				if (sym.n_type&N_EXT)
					c = toupper(c);
				sym.n_type = c;
				symp[i++] = sym;
			}
			if (nosort_flg==0)
				qsort(symp, i, sizeof(struct nlist), compare);
			if ((arch_flg || narg>1) && prep_flg==0)
				printf("\n%s:\n", SELECT);
			for (n=0; n<i; n++) {
				if (prep_flg) {
					if (arch_flg)
						printf("%s:", *argv);
					printf("%s:", SELECT);
				}
				c = symp[n].n_type;
				if (!undef_flg) {
					if (c=='u' || c=='U')
						printf("      ");
					else
						printf(FORMAT, symp[n].n_value);
					printf(" %c ", c);
				}
				printf("%.8s\n", symp[n].n_name);
			}
			if (symp) {
				free((char *)symp);
				symp = 0;
			}
		} while(arch_flg && nextel(fi));
		fclose(fi);
	}
	exit(!didone);
}

compare(p1, p2)
struct nlist *p1, *p2;
{
	register i;

	if (numsort_flg) {
		if (p1->n_value > p2->n_value)
			return(revsort_flg);
		if (p1->n_value < p2->n_value)
			return(-revsort_flg);
	}
	for(i=0; i<sizeof(p1->n_name); i++)
		if (p1->n_name[i] != p2->n_name[i]) {
			if (p1->n_name[i] > p2->n_name[i])
				return(revsort_flg);
			else
				return(-revsort_flg);
		}
	return(0);
}

nextel(af)
FILE *af;
{
	register char *cp;
	register r;

	fseek(af, off, 0);
	r = fread((char *)&arp, 1, sizeof(struct ar_hdr), af);  /* read archive header */
	if (r != sizeof(struct ar_hdr))
		return(0);
	for (cp = arp.ar_name; cp < &arp.ar_name[sizeof(arp.ar_name)]; cp++)
		if (*cp == ' ')
			*cp = '\0';
	arsize = atol(arp.ar_size);
	if (arsize & 1)
		++arsize;
	off = ftell(af) + arsize;	/* offset to next element */
	return(1);
}
