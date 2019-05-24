static	char *sccsid = "@(#)arff.c	4.7 (Berkeley) 81/07/08";
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#define dbprintf printf
struct rt_dat {
unsigned short	int	rt_yr:5;	/* Year - 1972 */
unsigned short	int	rt_dy:5;	/* day */
unsigned short	int	rt_mo:5;	/* month */
};
struct	rt_axent {
	char	rt_sent[14];
};

struct rt_ent {
	char  rt_pad;			/* unusued */
	char  rt_stat;			/* Type of entry, or end of seg */
	unsigned short rt_name[3];	/* Name, 3 words in rad50 form */
	short rt_len;			/* Length of file */
	char  rt_chan;			/* Only used in temporary files */
	char  rt_job;			/* Only used in temporary files */
	struct rt_dat rt_date;		/* Creation Date */
};
#define RT_TEMP 1
#define RT_NULL 2
#define RT_FILE 4
#define RT_ESEG 8
#define RT_BLOCK 512
#define RT_DIRSIZE 31			/* max # of directory segments */
struct rt_head {
	short	rt_numseg;		/* number of segments available */
	short	rt_nxtseg;		/* segment no of next log. seg */
	short	rt_lstseg;		/* highest seg currenltly open */
	unsigned short	rt_entpad;	/* extra words/dir. entry      */
	short	rt_stfile;		/* block no where files begin  */
};
struct	rt_dir {
	struct rt_head	rt_axhead;
	struct rt_ent	rt_ents[72];
	char	_dirpad[6];
};
extern struct rt_dir	rt_dir[RT_DIRSIZE];
extern int		rt_entsiz;
extern int		floppydes;
extern char		*rt_last;
typedef struct fldope {
	int	startad;
	int	count;
struct	rt_ent	*rtdope;
} FLDOPE;
#define	FL_BLOCK 128
FLDOPE *lookup();
#define rt(p) ((struct rt_ent *) p )
#define Ain1 03100
#define Ain2 050
#define flag(c) (flg[(c) - 'a'])

char	*man	=	{ "rxtd" };

char zeroes[512];
extern char *val;
extern char table[256];
struct rt_dir	
   rt_dir[RT_DIRSIZE] = {{{4,0,1,0,14},{0,RT_NULL,{0,0,0},494,0}, {0,RT_ESEG}}};
int		rt_entsiz;
int		rt_nleft;
struct rt_ent	*rt_curend[RT_DIRSIZE];
int		floppydes;
int		dirdirty;
char		*rt_last;
char		*defdev = "/dev/floppy";

char	*opt	=	{ "vf" };

int	signum[] = {SIGHUP, SIGINT, SIGQUIT, 0};
long	lseek();
int	rcmd();
int	dcmd();
int	xcmd();
int	tcmd();
int	(*comfun)();
char	flg[26];
char	**namv;
int	namc;
int	file;


main(argc, argv)
char *argv[];
{
	register char *cp;

	/* register i;
	for(i=0; signum[i]; i++)
		if(signal(signum[i], SIG_IGN) != SIG_IGN)
			signal(signum[i], sigdone); */
	if(argc < 2)
		usage();
	cp = argv[1];
	for(cp = argv[1]; *cp; cp++)
	switch(*cp) {
	case 'm':
	case 'v':
	case 'u':
	case 'w':
		flg[*cp - 'a']++;
		continue;
	case 'c':
		{
#define SURE	"Are you sure you want to clobber the floppy?\n"
			int tty;
			char response[128];
			tty = open("/dev/tty",2);
			write(tty,SURE,sizeof(SURE));
			read(tty,response,sizeof(response));
			if(*response!='y')
				exit(50);
			flag('c')++;
			close(tty);
		}
		dirdirty++;
		continue;

	case 'r':
		setcom(rcmd);
		flag('r')++;
		continue;

	case 'd':
		setcom(dcmd);
		flag('d')++;
		continue;

	case 'x':
		setcom(xcmd);
		continue;

	case 't':
		setcom(tcmd);
		continue;

	case 'f':
		defdev = argv[2];
		argv++;
		argc--;
		continue;


	default:
		fprintf(stderr, "arff: bad option `%c'\n", *cp);
		exit(1);
	}
	namv = argv+2;
	namc = argc-2;
	if(comfun == 0) {
		if(flag('u') == 0) {
			fprintf(stderr, "arff: one of [%s] must be specified\n", man);
			exit(1);
		}
		setcom(rcmd);
	}
	(*comfun)();
	exit(notfound());
}

setcom(fun)
int (*fun)();
{

	if(comfun != 0) {
		fprintf(stderr, "arff: only one of [%s] allowed\n", man);
		exit(1);
	}
	comfun = fun;
}

usage()
{

	fprintf(stderr, "usage: arff [%s][%s] archive files ...\n", opt, man);
	exit(1);
}

notfound()
{
	register i, n;

	n = 0;
	for(i=0; i<namc; i++)
		if(namv[i]) {
			fprintf(stderr, "arff: %s not found\n", namv[i]);
			n++;
		}
	return(n);
}

phserr()
{

	fprintf(stderr, "arff: phase error on %s\n", file);
}

mesg(c, name)
char *name;
{

	if(flag('v'))
		if(c != 'c' || flag('v') > 1)
			printf("%c - %s\n", c, name);
}

tcmd()
{
	register char *de;
	int segnum;
	register char *last;
	FLDOPE *lookup(), *dope;
	int nleft; register i;
	register struct rt_ent *rde;

	rt_init();
	if(namc==0)
	    for (segnum=0; segnum >= 0;    /* for all dir. segments */
		 segnum = rt_dir[segnum].rt_axhead.rt_nxtseg - 1) {
		last = rt_last + segnum*2*RT_BLOCK;
		for(de=((char *)&rt_dir[segnum])+10; de <= last; 
		    de += rt_entsiz) {
			if(rtls(rt(de))) {
				nleft = (last - de) / rt_entsiz;
				printf("\n%d entries remaining",nleft);
				printf(" in directory segment %d.\n",segnum+1);
				break;
			}
		}
	    }
	else
		for(i = 0; i < namc; i++) {
			if(dope = lookup(namv[i])) {
				rde = dope->rtdope;
				rtls(rde);
				namv[i] = 0;
			}
		}
}

rtls(de)
register struct rt_ent *de;
{
	int month,day,year;
	char name[12], ext[4];

	if(flag('v'))
		switch(de->rt_stat) {
		case RT_TEMP:
			printf("Tempfile:\n");
		case RT_FILE:
			unrad50(2,de->rt_name,name);
			unrad50(1,&(de->rt_name[2]),ext);
			day = de->rt_date.rt_dy;
			year = de->rt_date.rt_yr + 72;
			month = de->rt_date.rt_mo;
			printf("%6.6s  %3.3s	%02d/%02d/%02d	%d\n",name,
				ext,month,day,year,de->rt_len);
			break;

		case RT_NULL:
			printf("%-25.9s	%d\n","<UNUSED>",de->rt_len);
			break;

		case RT_ESEG:
			return(1);
		}
	else {
		switch(de->rt_stat) {
		case RT_TEMP:
		case RT_FILE:
			sunrad50(name,de->rt_name);
			printf(name);putchar('\n');
			break;

		case RT_ESEG:
			return(1);

		case RT_NULL:
			break;
		}
	}
	return(0);
}
xcmd()
{
	register char *de;
	int segnum;
	register char *last;
	char name[12];
	register int i;

	rt_init();
	if(namc==0)
	    for (segnum=0; segnum >= 0;    /* for all dir. segments */
		 segnum = rt_dir[segnum].rt_axhead.rt_nxtseg - 1) {
		last = rt_last + segnum*2*RT_BLOCK;
		for(de=((char *)&rt_dir[segnum])+10; de <= last; 
		    de += rt_entsiz) {
			sunrad50(name,rt(de)->rt_name);
			rtx(name);
		}
	    }
	else
		for(i = 0; i < namc; i++)
		if(rtx(namv[i])==0) namv[i] = 0;
}
rtx(name)
char *name;
{
	register FLDOPE *dope;
	FLDOPE *lookup();
	register startad, count;
	int file; char buff[RT_BLOCK];


	if(dope = lookup(name)) {
		mesg('x', name);
		file = creat(name, 0666);
		if(file < 0) return(1);
		count = dope->count;
		startad = dope->startad;
		for( ; count > 0 ; count -= RT_BLOCK) {
			lread(startad,RT_BLOCK,buff);
			write(file,buff,RT_BLOCK);
			startad += RT_BLOCK;
		}
		close(file);
		return(0);
	}
	return(1);
}
rt_init()
{
	static initized = 0;
	register char *de;
	register i;
	int dirnum;
	char *mode;
	register char *last;
	FILE *temp_floppydes;

	if(initized) return;
	initized = 1;
	if(flag('c') || flag('d') || flag('r'))
		mode = "r+";
	else
		mode = "r";
	if((temp_floppydes = fopen(defdev, mode)) == NULL) {
		perror(defdev);
		exit(1);
	}
	floppydes = fileno(temp_floppydes);
	if(flag('c')==0) {
		lread(6*RT_BLOCK,2*RT_BLOCK,(char *)&rt_dir[0]);
		dirnum = rt_dir[0].rt_axhead.rt_numseg;
		if (dirnum > RT_DIRSIZE) {
		   fprintf(stderr,"arff: too many directory segments\n");
		   exit(1);
		}
		for (i=1; i<dirnum; i++)
		   lread((6+2*i)*RT_BLOCK,2*RT_BLOCK,(char *)&rt_dir[i]);
	} else
		dirnum = 1;

	rt_entsiz = 2*rt_dir[0].rt_axhead.rt_entpad + 14;
	rt_entsiz = 14;			/* assume rt_entpad = 0 ??? */
	rt_last = ((char *) &rt_dir[0]) + 10 + 1014/rt_entsiz*rt_entsiz; 
	rt_nleft = 0;
	
	for (i=0; i<dirnum; i++) {
	    if (rt_dir[i].rt_axhead.rt_nxtseg >= RT_DIRSIZE
	    ||  rt_dir[i].rt_axhead.rt_lstseg >= RT_DIRSIZE) {
		fprintf(stderr, "arff: awful directory linkage\n");
		exit(1);
	    }
  	    last = rt_last + i*2*RT_BLOCK;
	    for(de=((char *)&rt_dir[i])+10; de <= last; de += rt_entsiz) {
		if(rt(de)->rt_stat==RT_ESEG) break;
	    }
	    rt_curend[i] = rt(de);
	    rt_nleft += (last - de) / rt_entsiz;
	}
}

static FLDOPE result;
FLDOPE *
lookup(name)
char * name;
{
	unsigned short rname[3];
	register char *de;
	int segnum;
	register char *last;
	register index;

	srad50(name,rname);

	/* 
	 *  Search for name, accumulate blocks in index
	 */
	rt_init();
	for (segnum=0; segnum >= 0;    /* for all dir. segments */
             segnum = rt_dir[segnum].rt_axhead.rt_nxtseg - 1) {
	    index = 0;
	    last = rt_last + segnum*2*RT_BLOCK;
	    for(de=((char *)&rt_dir[segnum])+10; 
		rt(de)->rt_stat != RT_ESEG; de += rt_entsiz) {
		switch(rt(de)->rt_stat) {
		case RT_FILE:
		case RT_TEMP:
		if(samename(rname,rt(de)->rt_name))
			goto found;
		case RT_NULL:
			index += rt(de)->rt_len;
		}
	    }
        }
	return((FLDOPE *) 0);
found:	result.count = rt(de)->rt_len * RT_BLOCK;
	result.startad = RT_BLOCK * (rt_dir[segnum].rt_axhead.rt_stfile + index);
	result.rtdope = (struct rt_ent *) de;
	return(&result);
}
static
samename(a,b)
unsigned short a[3],b[3];
{
	return( a[0]==b[0] && a[1]==b[1] && a[2]==b[2] );
}


rad50(cp,out)
register unsigned char *cp;
unsigned short *out;
{
	register index;
	register temp;

	for(index = 0;*cp; index++) {

		temp = Ain1 * table[*cp++];
		if(*cp!=0) {
			temp += Ain2 * table[*cp++];

			if(*cp!=0) 
				temp += table[*cp++];
		}

		out[index] = temp;
	}
}
#define reduce(x,p,q) \
	(x = v[p/q], p %= q);

unrad50(count,in,cp)
unsigned short *in;
register char *cp;
{
	register i, temp; register unsigned char *v = (unsigned char *) val;
	
	for(i = 0; i < count; i++) {
		temp = in[i];

		reduce (*cp++,temp,Ain1);
		reduce (*cp++,temp,Ain2);
		reduce (*cp++,temp,1);
	}
	*cp=0;
}

srad50(name,rname)
register char * name;
register unsigned short *rname;
{
	register index; register char *cp;
	char file[7],ext[4];
	/* 
	 * Find end of pathname
	 */
	for(cp = name; *cp++; );
	while(cp >= name && *--cp != '/');
	cp++;
	/* 
	 * Change to rad50
	 *
	 */
	for(index = 0; *cp; ){
		file[index++] = *cp++;
		if(*cp=='.') {
			cp++;
			break;
		}
		if(index>=6) {
			break;
		}
	}
	file[index] = 0;
	for(index = 0; *cp; ){
		ext[index++] = *cp++;
		if(*cp=='.' || index>=3) {
			break;
		}
	}
	ext[index]=0;
	rname[0] = 0;
	rname[1] = 0;
	rname[2] = 0;
	rad50((unsigned char *)file,rname);
	rad50((unsigned char *)ext,rname+2);
}
sunrad50(name,rname)
unsigned short rname[3];
register char *name;
{
	register char *cp, *cp2;
	char ext[4];

	unrad50(2,rname,name);
	unrad50(1,rname + 2,ext);
	/* Jam name and extension together with a dot
	   deleting white space */
	for(cp = name; *cp++;);--cp;  while(*--cp==' ' && cp>=name);
	*++cp = '.';cp++;
	for(cp2=ext; *cp2!=' ' && cp2 < ext + 3;) {
		*cp++ = *cp2++;
	}
	*cp=0;
	if(cp[-1]=='.') cp[-1] = 0;
}

static char *oval = " ABCDEFGHIJKLMNOPQRSTUVWXYZ$.@0123456789";
static char *val = " abcdefghijklmnopqrstuvwxyz$.@0123456789";
static char table[256] = {
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
0, 29, 29, 29, 27, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 29, 
30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 29, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29, 29, 
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 
0, 29, 29, 29, 27, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 29, 
30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 29, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29, 29, 
29, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 29, 29, 29, 29 };
		
long
trans(logical)
register int logical;
{
	/*  Logical to physical adress translation */
	register int sector, bytes, track;

	logical += 26 * 128;
	bytes = (logical & 127);
	logical >>= 7;
	sector = logical % 26;
	if(sector >= 13)
		sector = sector *2 +1;
	else
		sector *= 2;
	sector += 26 + ((track = (logical / 26)) - 1) * 6;
	sector %= 26;
	return( (((track *26) + sector) << 7) + bytes);
}
lread(startad,count,obuff)
register startad, count;
register char * obuff;
{
	long trans();
	extern floppydes;
	int chunk;

	rt_init();
	if(flag('m')==0)
		chunk = FL_BLOCK;
	else
		chunk = RT_BLOCK;
	for (; count > 0; count -= chunk, obuff += chunk, startad += chunk) {
		lseek(floppydes,
		    (long) (flag('m') ? startad : trans(startad)), 0);
		if (read(floppydes, obuff, chunk) != chunk)
			fprintf(stderr, "arff: read error, block %d\n", startad / RT_BLOCK);
	}
}

lwrite(startad,count,obuff)
register startad, count;
register char * obuff;
{
	long trans();
	extern floppydes;
	int chunk;

	rt_init();
	if(flag('m')==0)
		chunk = FL_BLOCK;
	else
		chunk = RT_BLOCK;
	for (; count > 0; count -= chunk, obuff += chunk, startad += chunk) {
		lseek(floppydes,
		    (long) (flag('m') ? startad : trans(startad)), 0);
		if (write(floppydes, obuff, chunk) != chunk)
			fprintf(stderr, "arff: write error, block %d\n", startad / RT_BLOCK);
	}
}

rcmd()
{
	register int i;

	rt_init();
	if (namc>0)
		for(i = 0; i < namc; i++)
			if(rtr(namv[i])==0) namv[i]=0;
}

rtr(name)
char *name;
{
	register FLDOPE *dope;
	struct stat buf;
	FLDOPE *flcreat();

	if(stat(name,&buf)<0) {
		perror(name);
		return (-1);
	}
	if ((dope = flcreat(name, buf.st_size, buf.st_mtime)) == NULL)
		return (-1);
	mesg('r', name);
	toflop(name, buf.st_size, dope);
	return (0);
}

FLDOPE *
flcreat(name, size, mtime)
char *name;
long size;
time_t mtime;
{
	register FLDOPE *dope;
	register struct rt_ent *de;
	int segnum;

	if ((dope = lookup(name)) != NULL
	&&  size <= dope->rtdope->rt_len * RT_BLOCK)
		return (dope);
	for (segnum=0; segnum != -1; segnum = rt_dir[segnum].rt_axhead.rt_nxtseg - 1) {
		for(de = rt_dir[segnum].rt_ents; de->rt_stat != RT_ESEG; de++)
			if (de->rt_stat == RT_NULL
			&&  size <= de->rt_len * RT_BLOCK) {
				if (dope)
					delete(dope);
				mkent(de, segnum, size, name, mtime);
				goto found;
			}
	}
	printf("%s: no slot or no space for file\n", name);
	return (NULL);
found:
	if (dope = lookup(name))
		return (dope);
	fprintf(stderr, "%s: internal error, added then not found\n", name);
	return (NULL);
}

mkent(de,segnum,size,name,mtime)
register struct rt_ent *de;
int segnum;
long size;
char *name;
time_t mtime;
{
	struct tm *localtime(); register struct tm *timp;
	register struct rt_ent *workp; int count;
	
	count = (((size - 1)/RT_BLOCK) + 1);
						/* Make sure there is room */
	if(de->rt_len==count)
		goto overwrite;
	if(rt_curend[segnum] == (rt_last + (segnum*2*RT_BLOCK))) {
						/* no entries left on segment */
		if(flag('o'))
			goto overwrite;
		fprintf(stderr,"Directory segment #%d full on  %s\n",segnum+1,
                    defdev);
		exit(1);
	}	
					/* copy directory entries up */
	for(workp = rt_curend[segnum]+1; workp > de; workp--)
		*workp = workp[-1];
	de[1].rt_len -= count;
	de->rt_len = count;
	rt_curend[segnum]++;
	rt_nleft--;
overwrite:
	srad50(name,de->rt_name);
	timp = localtime(&mtime);
	de->rt_date.rt_dy = timp->tm_mday + 1;
	de->rt_date.rt_mo = timp->tm_mon + 1;
	de->rt_date.rt_yr = timp->tm_year - 72;
	de->rt_stat = RT_FILE;
	de->rt_pad = 0;
	de->rt_chan = 0;
	de->rt_job = 0;
	lwrite((6+segnum*2)*RT_BLOCK,2*RT_BLOCK,(char *)&rt_dir[segnum]);
}

toflop(name,ocount,dope)
char *name;
register FLDOPE *dope;
long ocount;
{
	register file, n, startad = dope->startad, count = ocount;
	char buff[RT_BLOCK];
	
	file = open(name,0);
	if(file < 0) {
		fprintf(stderr, "arff: couldn't open %s\n",name);exit(1);}
	for( ; count >= RT_BLOCK; count -= RT_BLOCK) {
		read(file,buff,RT_BLOCK);
		lwrite(startad,RT_BLOCK,buff);
		startad += RT_BLOCK;
	}
	read(file,buff,count);
	close(file);
	if(count <= 0) return;
	for(n = count; n < RT_BLOCK; n ++) buff[n] = 0;
	lwrite(startad,RT_BLOCK,buff);
	count = (dope->rtdope->rt_len * RT_BLOCK - ocount) / RT_BLOCK ;
	if(count <= 0) return;
	for( ; count > 0 ; count--) {
		startad += RT_BLOCK;
		lwrite(startad,RT_BLOCK,zeroes);
	}
}
dcmd()
{
	register int i;

	rt_init();
	if(namc)
		for(i = 0; i < namc; i++)
			if(rtk(namv[i])==0) namv[i]=0;
	if(dirdirty)
		scrunch();
	
}
rtk(name)
char *name;
{
	FLDOPE *dope;
	register struct rt_ent *de;
	FLDOPE *lookup();

	if(dope = lookup(name)) {
		mesg('d', name);
		delete(dope);
		return(0);
	}
	return(1);
}

delete(dope)
FLDOPE *dope;
{
	register struct rt_ent *de;

	de = dope->rtdope;
	de->rt_stat = RT_NULL;
	de->rt_name[0] = 0;
	de->rt_name[1] = 0;
	de->rt_name[2] = 0;
	* ((unsigned short *) & (de->rt_date)) = 0;
	dirdirty = 1;
}

scrunch() {
	register struct rt_ent *de , *workp;
	register segnum;
	for (segnum=0; segnum != -1;    /* for all dir. segments */
	     segnum = rt_dir[segnum].rt_axhead.rt_nxtseg - 1) {
	    dirdirty = 0;
	    for(de = rt_dir[segnum].rt_ents; de <= rt_curend[segnum]; de++) {
		if(de->rt_stat==RT_NULL && de[1].rt_stat==RT_NULL) {
			(de+1)->rt_len += de->rt_len;
			for(workp = de; workp < rt_curend[segnum]; workp++)
				*workp = workp[1];
			de--;
			rt_curend[segnum]--;
			rt_nleft++;
			dirdirty = 1;
		}
	    }
	    if (dirdirty)
	    lwrite((6+segnum*2)*RT_BLOCK,2*RT_BLOCK,(char *)&rt_dir[segnum]);
	}
}
