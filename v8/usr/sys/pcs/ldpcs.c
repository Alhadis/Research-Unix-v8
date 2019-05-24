/*
 * load comet microcode patches
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/mtpr.h>

typedef long preg_t;		/* type of a processor register */
typedef long word_t;		/* type of a `word' in pcs */

/*
 * format of the patch file, as distributed by DEC
 * first 1k bytes are patch bits:
 *	each bit goes in a successive 32-bit word
 * remaining 10k bytes are the microcode proper
 *	each 20 bits goes in a successive 32-bit word
 */

#define	FPATLEN	1024	/* number of bytes of packed patch bit */
#define	FMICLEN	10240	/* number of bytes of packed microcode */

#define	NPAT1	(FPATLEN*8)	/* number of patch bits */
#define	NMIC20	((FMICLEN*8)/20)	/* number of 20-bit microcode lumps */

word_t pat[NPAT1];
word_t mic[NMIC20]; 

/*
 * PCS definitions
 */

#define	PCSPAT	0xf00000	/* first patchbit loc */
#define	PCSMIC	0xf08000	/* first microcode loc */
#define	PCSENA	0xf0c000	/* patchbit enable addr */
#define	ENABLE	0xfff00000	/* bits to enable pcs */

#define	MINVER	0x5f		/* minimum ucode version supporting pcs */
#define	STYPE	0xff000000	/* processor type in SID */
#define	SCOMET	0x02000000	/* VAX-11/750 */

int fmem, freg;
int force = 0;
int babble = 0;

main(argc, argv)
int argc;
char **argv;
{

	while (--argc > 0 && **++argv == '-') {
		if (strcmp(argv[0], "-f") == 0)
			force++;
		else if (strcmp(argv[0], "-v") == 0)
			babble++;
		else
			usage();
	}
	if (argc != 1)
		usage();
	openmem();
	if (force == 0 && micver() < MINVER) {
		fprintf(stderr, "no pcs hardware");
		exit(1);
	}
	getpcs(argv[0]);
	loadpcs();
	if (babble)
		printf("ucode rev x%x\n", micver());
	exit(0);
}

usage()
{

	fprintf(stderr, "usage: ldpcs [ -f ] [ -v ] pcsfile\n");
	exit(1);
}

openmem()
{

	if ((fmem = open("/dev/mem", 2)) < 0) {
		perror("/dev/mem");
		exit(1);
	}
	if ((freg = open("/dev/mtpr", 2)) < 0) {
		perror("/dev/mtpr");
		exit(1);
	}
}

micver()
{
	preg_t sid;
	preg_t mfpr();

	sid = mfpr(SID);
	if (force == 0 && (sid & STYPE) != SCOMET) {
		fprintf(stderr, "not a VAX-11/750\n");
		exit(1);
	}
	return ((sid >> 8) & 0xff);
}

/*
 * get the patch bits and microcode out of the file
 * leave them unpacked and ready to load in pat and mic
 */

getpcs(f)
char *f;
{
	int fd;
	char ppat[FPATLEN];
	char pmic[FMICLEN];
	char junk;

	if ((fd = open(f, 0)) < 0) {
		perror(f);
		exit(1);
	}
	if (read(fd, ppat, sizeof(ppat)) != sizeof(ppat)) {
		fprintf(stderr, "%s: can't read patch bits\n", f);
		close(fd);
		exit(1);
	}
	if (read(fd, pmic, sizeof(pmic)) != sizeof(pmic)) {
		fprintf(stderr, "%s: can't read microcode\n", f);
		close(fd);
		exit(1);
	}
	/* sanity check */
	if (read(fd, &junk, 1) != 0) {
		fprintf(stderr, "%s: bad format\n", f);
		close(fd);
		exit(1);
	}
	close(fd);
	unpack(ppat, pat, NPAT1, 1);
	unpack(pmic, mic, NMIC20, 20);
}

/*
 * load the PCS:
 *	for safety, turn off any ucode now running there
 *	enable patching; write patch bits; disable patching
 *	write microcode
 *	enable the PCS, i.e. turn it on
 */

loadpcs()
{
	long w;

	w = 0;
	mwrite(fmem, (off_t)PCSMIC, (char *)&w, sizeof(w));	/* disable pcs */
	w = 1;
	mwrite(fmem, (off_t)PCSENA, (char *)&w, sizeof(w));	/* enable patching */
	mwrite(fmem, (off_t)PCSPAT, (char *)pat, sizeof(pat));	/* write patch bits */
	w = 0;
	mwrite(fmem, (off_t)PCSENA, (char *)&w, sizeof(w));	/* disable patching */
	mwrite(fmem, (off_t)PCSMIC, (char *)mic, sizeof(mic));	/* write microcode */
	w = mic[0] | ENABLE;
	mwrite(fmem, (off_t)PCSMIC, (char *)&w, sizeof(w));	/* enable pcs */
}

mwrite(fd, addr, buf, size)
int fd;
off_t addr;
char *buf;
int size;
{

	lseek(fd, addr, 0);
	if (write(fd, buf, size) != size) {
		perror("write");
		exit(1);
	}
}

preg_t
mfpr(rno)
int rno;
{
	long reg;

	lseek(freg, (off_t)rno * sizeof(reg), 0);
	if (read(freg, (char *)&reg, sizeof(reg)) != sizeof(reg)) {
		perror("mtpr");
		exit(1);
	}
	return (reg);
}
