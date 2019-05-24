/*
 * MC68000 object code optimizer
 *   -- produce short branches
 *   -- shorten address references
 *   -- If I get the energy, get rid of branch-to-branch
 */

#include <stdio.h>
#include "/usr/blit/include/a.out.h"

typedef	unsigned short	INS;
#define	SINS	sizeof(INS)
#define	STSIZE	8192
#define	XLOC	0100000			/* a squashed address */


INS	*memalloc();
long	newloc();
INS	*textb;
INS	*texte;
INS	*relb;
INS	*rele;
long	*stab;
long	*ste;
long	*stb;
struct	exec hdr;
struct	exec nhdr;
int	infile;
int	outfile;
int	saflag;
int	startloc;

/* # words in address mode, indexed by 6-bit EA */
/* immediate (74) assumes byte/short */
char	adrsize[] = {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	1,	1,	1,	1,	1,	1,	1,	1,
	1,	1,	1,	1,	1,	1,	1,	1,
	1,	2,	1,	1,	1,	0,	0,	0,
};

main(argc, argv)
char **argv;
{
	char *infname = NULL, *outfname = NULL;

	while (argc > 1) {
		if (argv[1][0]=='-') {
			if (argv[1][1]=='o') {
				if (argv[1][2]) {
					outfname = &argv[1][2];
					argc--;
					argv++;
					continue;
				} else if (argc > 2) {
					outfname = argv[2];
					argc -= 2;
					argv += 2;
					continue;
				} else
					error("no output file specified", NULL);
			} else
				error("mopt [ -o output ] input", NULL);
		}
		if (infname)
			error("only one input file allowed", NULL);
		infname = argv[1];
		argc--;
		argv++;
	}
	if (infname==NULL)
		infname = "a.out";
	input(infname);
	process1();
	process2(0);
	nhdr.a_text -= SINS*(ste-stab);
	if (outfname==NULL)
		outfname = "m.out";
	output(infname, outfname);
	exit(0);
}

input(f)
char *f;
{
	if ((infile = open(f, 0)) < 0)
		error("cannot open %s", f);
	if (read(infile, (char *)&hdr, sizeof(hdr)) != sizeof(hdr))
		error("%s has no header", f);
	nhdr = hdr;
	switch (hdr.a_magic) {

	case A_MAGIC1:
		saflag++;
		startloc = 256;		/* hack */
		break;

	case A_MAGIC4:
		break;

	default:
		error("%s is not a suitable object file", f);
	}
	if (hdr.a_flag)
		error("%s has no relocation information", f);
	textb = memalloc(hdr.a_text, "text");
	texte = (INS *)((char *)textb + hdr.a_text);
	if (read(infile, (char *)textb, hdr.a_text) != hdr.a_text)
		error("cannot find all of text for %s", f);
	relb = memalloc(hdr.a_text, "text reloc");
	rele = (INS *)((char *)relb + hdr.a_text);
	lseek(infile, sizeof(hdr)+hdr.a_text+hdr.a_data, 0);
	if (read(infile, (char *)relb, hdr.a_text) != hdr.a_text)
		error("cannot find all of reloc for %s", f);
	stab = (long *)memalloc(STSIZE*sizeof(long), "shrink table");
	ste = stab;
	stb = &stab[STSIZE];
}

process1()
{
	register INS *tp, *rp;
	register long loc;
	int totbr=0, shrinkbr=0, totea=0, shrinkab=0, shrinkpc=0,  totim=0;
	int totsbr=0, nbsr=0;
	int mm0=0, mm1=0;

	for (tp=textb,rp=relb,loc=startloc; tp<texte; tp++,rp++,loc+=SINS) {
		register off = 0;
		register long addr;

		if ((*rp&N_TYPE) == N_ABS)
		switch (*rp & TMASK) {

		case TBR0:
			totsbr++;
			continue;

		case TBR1:
			totbr++;
			squeeze(loc+SINS);
			addr = newloc(loc+(short)tp[1]+SINS) - newloc(loc) - SINS;
			if (addr < 128 && addr >= -128
			 && addr != 0 && rp[1]==XPCREL+N_TEXT)
				shrinkbr++;
			else
				*--ste = NULL;
			continue;

		case TEA2:
		case TPC2:
			off++;
		case TEA1:
		case TPC1:
			off++;
		case TEA0:
		case TPC0:
			off++;
			if ((*tp & 077) == 071) {	/* long abs */
				totea++;
				addr = (tp[off]<<16) | tp[off+1];
				if (((rp[off] & N_TYPE) == (N_ABS|X2WDS)
				   /*|| saflag*/)
				 && addr <= 32767 && addr >= -32767) {
					shrinkab++;
					squeeze(loc+SINS*off);
				} else if ((*rp & TMASK) >= TPC0)  {
					addr -= loc;
					if (addr <= 32767 && addr >= -32767) {
						shrinkpc++;
						squeeze(loc+SINS*off);
						/* jsr->bsr ; a fun frill */
						if (addr<128 && addr>=-128
						 && (*tp&0177700)==0047200) {
							nbsr++;
							squeeze(loc+SINS*(off+1));
						}
					}
				}
			}
			break;

		case TIM0:
			addr = (tp[1]<<16) + tp[2];
			if (((rp[1] & N_TYPE) == (N_ABS|X2WDS)
			   /*|| saflag*/)
			 && addr<=32767 && addr>=-32767) {
				totim++;
				squeeze(loc+SINS);
			}
			continue;

		case TMM:		/* move multiple */
			if (tp[1]==0) {	/* 0 regs */
				/* toss instruction, mask, address */
				for (off=0; off<adrsize[*tp&077]+2; off++) {
					squeeze(loc+off*SINS);
					mm0++;
				}
			} else if (((tp[1]-1) & tp[1]) == 0)	/* 1 reg */
				mm1++;
			break;
		}
		off = 0;
		switch (*rp & TMASK1) {

		case TEA2<<4:
			off++;
		case TEA1<<4:
			off++;
		case TEA0<<4:
			off++;
			if ((*tp & 07700) == 01700) {	/* long abs for mov dest */
				totea++;
				addr = (tp[off]<<16) | tp[off+1];
				if (addr <= 32767 && addr >= -32767
				 && ((rp[off] & N_TYPE) == (N_ABS|X2WDS)
				    /*|| saflag*/)) {
					shrinkab++;
					squeeze(loc+off*SINS);
				}
			}
			break;
		}
	}
	printf("%d branch, %d shrink\n", totbr, shrinkbr);
	printf("%d long ea, %d pcshrink %d abshrink\n", totea, shrinkpc, shrinkab);
	printf("%d immed\n", totim);
	printf("%d short br\n", totsbr);
	printf("%d bsr\n", nbsr);
	printf("%d mm0, %d mm1\n", mm0, mm1);
	printf("save %.1f%%\n",
	 200.0*(nbsr+mm0+shrinkab+shrinkpc+shrinkbr+totim)/hdr.a_text);
}

process2(tordata)
{
	register INS *tp, *rp, *ntp, *nrp;
	register long loc;
	register long *stp;

	stp = stab;
	loc = startloc;
	if (tordata) {
		stp = ste;
		loc = nhdr.a_text + startloc;
	}
	for (tp=ntp=textb, rp=nrp=relb;
	     tp<texte; tp++, rp++, loc+=SINS) {
		register off;
		register long addr;
		register offfix;

		if (loc > *stp && stp < ste) {
			printf("At location %o, ", loc);
			printf("last, cur stp %o %o, ", stp[-1], stp[0]);
			error("botch-- missed shrinktable entry", (char *)0);
		}
		off = 0;
		offfix = 0;
		switch (*rp & TMASK) {

		case TBR1:
			addr = newloc(loc+(short)tp[1]+SINS) - newloc(loc) - SINS;
			if (addr < 128 && addr >= -128
			 && *stp == loc+SINS
			 && addr != 0 && rp[1]==XPCREL+N_TEXT) {
				*ntp++ = (*tp++&0xff00) | (addr&0xff);
				*nrp++ = N_ABS|TBR0;
				rp++;
				stp++;
				loc += SINS;
				continue;
			}
			break;

		case TBR0:
			off = *tp & 0xff;
			if (off & 0x80)
				off |= 0xffffff00;	/* sign-extend */
			addr = newloc(loc+SINS+off) - newloc(loc) - SINS;
			if (addr==0 || addr>=128 || addr<-128)
				error("Botch relocating short branch", (char *)0);
			*tp = *tp&0xff00 | (addr&0xff);
			*ntp++ = *tp;
			*nrp++ = *rp;
			continue;

		case TEA2:
		case TPC2:
			off++;
		case TEA1:
		case TPC1:
			off++;
		case TEA0:
		case TPC0:
			off++;
			if ((*tp & 077) == 071) {	/* long abs */
				addr = (tp[off]<<16) | tp[off+1];
				if (((rp[off] & N_TYPE) == (N_ABS|X2WDS)
				   /*|| saflag*/)
				 && addr <= 32767 && addr >= -32767
				 && *stp == loc+SINS*off) {
					register i;
					*tp = (*tp&~077)|070; /* short */
					*rp &= ~TMASK;
					rp[off+1] =
					  XLOC | rp[off]&N_TYPE&~X2WDS;
					if ((*rp & TMASK1) >= (TEA0<<4)
					 && (*rp & TMASK1) <= (TPC2<<4))
						*rp -= 04000;	/* fiddle mov dest */
					for (i=off-1; i>=0; i--) {
						tp[i+1] = tp[i];
						rp[i+1] = rp[i];
					}
					stp++;
					tp++;
					rp++;
					offfix = 1;
				} else if ((*rp & TMASK) >= TPC0
				 && (addr=newloc(addr)-newloc(loc)-SINS*off)<=32767
				 && addr >= -32767
				 && *stp == loc+SINS*off) {
					register i;

					/* first check jsr->bsr */
					if ((*tp & 0177700)==0047200
					 && addr<128 && addr>=-128
					 && *stp==loc+SINS && stp[1]==loc+2*SINS) {
						*ntp++ = 0060400 + (addr & 0377);
						*nrp++ = TBR0 + N_ABS;
						tp += 2;
						rp += 2;
						stp += 2;
						loc += 2*SINS;
						continue;
					}
					*tp = (*tp&~077)|072; /* pc rel */
					*rp -= *rp&TMASK;
					tp[off+1] -= loc + SINS*off ;
					rp[off+1] =
					 XLOC | XPCREL | rp[off]&N_TYPE&~X2WDS;
					if ((*rp & TMASK1) >= (TEA0<<4)
					 && (*rp & TMASK1) <= (TPC2<<4))
						*rp -= 04000;	/* fiddle mov dest */
					for (i=off-1; i>=0; i--) {
						tp[i+1] = tp[i];
						rp[i+1] = rp[i];
					}
					stp++;
					tp++;
					rp++;
					offfix = 1;
				}
			}
			break;

		case TIM0:
			addr = (tp[1]<<16) + tp[2];
			if (((rp[1] & N_TYPE) == (N_ABS|X2WDS)
			   /*|| saflag*/)
			 && addr<=32767 && addr>=-32767
			 && *stp == loc+SINS) {
				stp++;
				/* detect mov.l, with different size field */
				if ((*tp&0070000) == 0020000)
					tp[1] = (*tp&~0070000)|0030000;
				else
					tp[1] = (*tp&~0700)|0300;
				rp[1] = *rp & ~TMASK;
				tp++;
				rp++;
				rp[1] |= XLOC;
				rp[1] &= ~X2WDS;
				offfix = 1;
			}
			break;

		case TMM:
			if (*stp == loc) {
				off = adrsize[*tp&077] + 2;
				tp += off-1;
				rp += off-1;
				loc += SINS*(off-1);
				stp += off;
				continue;
			}
		}
		off = 0;
		switch (*rp & TMASK1) {

		case TEA2<<4:
			off++;
		case TEA1<<4:
			off++;
		case TEA0<<4:
			off++;
			if ((*tp & 07700) == 01700) {	/* long abs for mov dest */
				addr = (tp[off]<<16) | tp[off+1];
				if (addr <= 32767 && addr >= -32767
				 && ((rp[off] & N_TYPE) == (N_ABS|X2WDS)
				   /*|| saflag*/)
				 && *stp == loc+SINS*(off+offfix)) {
					register i;
					*tp = (*tp&~07700)|00700; /* short */
					*rp -= *rp&TMASK1;
					rp[off+1] = rp[off]&N_TYPE&~X2WDS;
					rp[off+1] |= XLOC;
					for (i=off-1; i>=0; i--) {
						tp[i+1] = tp[i];
						rp[i+1] = rp[i];
					}
					stp++;
					tp++;
					rp++;
				}
			}
			break;
		}
		switch (*rp & (N_TYPE|XPCREL|X2WDS)) {

		case N_TEXT|X2WDS:
		case N_DATA|X2WDS:
		case N_BSS|X2WDS:
			addr = newloc(((long)tp[0] << 16) | tp[1]);
			tp[0] = addr>>16;
			tp[1] = addr;
			break;

		case N_TEXT|XPCREL:
		case N_DATA|XPCREL:
		case N_BSS|XPCREL:
		case N_ABS|XPCREL:
			tp[0] = newloc(loc+(short)tp[0]) - newloc(loc);
			break;

		case N_ABS:
		case N_ABS|X2WDS:
			break;

		case N_TEXT:
		case N_DATA:
		case N_BSS:
			if (saflag) {
				tp[0] = newloc((short)tp[0]);
				break;
			}
			/* flow through */
		default:
			printf("at loc %o, text %o\n", loc, *tp);
			error("botch-- relocation %o", *rp);
			break;
		}
		if (*rp & XLOC) {
			loc += SINS;
			*rp &= ~XLOC;
		}
		*ntp++ = *tp;
		*nrp++ = *rp;
	}
	if (stp != ste)
		error("Didn't finish shrink table", (char *)0);
}

look(tp, rp)
register INS *tp, *rp;
{
	printf("%6.6o  %6.6o\n", *tp++, *rp++);
	printf("%6.6o  %6.6o\n", *tp++, *rp++);
	printf("%6.6o  %6.6o\n", *tp++, *rp++);
	printf("%6.6o  %6.6o\n", *tp++, *rp++);
	printf("%6.6o  %6.6o\n", *tp++, *rp++);
	printf("\n");
}

output(in, out)
char *in, *out;
{
	register struct nlist *symp;
	register i;

	if (strcmp(in, out)==0)
		if (unlink(in) < 0)
			error("cannot reuse %s", in);
	outfile = creat(out, 0666);
	if (outfile < 0)
		error("cannot create %s", out);
	cwrite((char *)&nhdr, (long)sizeof(nhdr));
	cwrite((char *)textb, nhdr.a_text);
	free((char *)textb);
	lseek(outfile, sizeof(hdr)+nhdr.a_text+nhdr.a_data, 0);
	cwrite((char *)relb, nhdr.a_text);
	free((char *)relb);
	textb = memalloc(hdr.a_data, "data");
	texte = (INS *)((char *)textb + hdr.a_data);
	lseek(infile, sizeof(hdr)+hdr.a_text, 0);
	if (read(infile, (char *)textb, hdr.a_data) != hdr.a_data)
		error("Cannot read all of data", (char *)NULL);
	relb = memalloc(hdr.a_data, "data reloc");
	rele = (INS *)((char *)relb + hdr.a_data);
	lseek(infile, sizeof(hdr)+2*hdr.a_text+hdr.a_data, 0);
	if (read(infile, (char *)relb, hdr.a_data) != hdr.a_data)
		error("Cannot read data reloc", (char *)NULL);
	process2(1);
	lseek(outfile, sizeof(hdr)+nhdr.a_text, 0);
	cwrite((char *)textb, hdr.a_data);
	lseek(outfile, sizeof(hdr)+2*nhdr.a_text+nhdr.a_data, 0);
	cwrite((char *)relb, hdr.a_data);
	free((char *)textb);
	free((char *)relb);
	textb = memalloc(hdr.a_syms, "symbols");
	if (read(infile, (char *)textb, hdr.a_syms) != hdr.a_syms)
		error("Cannot read symbols", (char *)NULL);
	i = 0;
	for (symp = (struct nlist *)textb;
	    (char *)symp < (char *)textb+hdr.a_syms; symp++) {
		register t = symp->n_type & N_TYPE;
		i++;
		if (t==N_TEXT || t==N_DATA || t==N_BSS)
			symp->n_value = newloc(symp->n_value);
		if (symp->n_type==0 && symp->n_dtype==0)
			printf("Funny symbol\n");
	}
	printf("%d symbols\n", i);
	cwrite((char *)textb, hdr.a_syms);
}

INS *
memalloc(n, t)
long n;
char *t;
{
	INS *ap;

	ap = (INS *)malloc((unsigned)n);
	if (ap == NULL)
		error("Cannot allocate memory for %s", t);
	return(ap);
}

/*
 * Map address to final address;
 *  binary search of the mapping table
 */
long
newloc(loc)
register loc;
{
	register hi, lo, new;

	lo = 0;
	hi = ste - stab - 1;
	while (lo <= hi) {
		new = (hi+lo)/2;
		if (loc > stab[new])
			lo = new+1;
		else
			hi = new-1;
	}
	return(loc - lo*SINS);
}

/*
 * The halfword at (old) pc loc will disappear
 */
squeeze(loc)
long loc;
{
	if (ste >= stb)
		error("No space for shrink table", (char *)0);
	*ste++ = loc;
}

cwrite(p, n)
char *p;
long n;
{
	if (write(outfile, p, n) != n)
		error("write error", (char *)NULL);
}

error(msg, cp)
char *msg, *cp;
{
	printf("mopt: ");
	printf(msg, cp);
	printf("\n");
	exit(1);
}
