/*	boot.c	4.6	81/12/01	*/

#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/vm.h>
#include <a.out.h>
#include "saio.h"
#include <sys/reboot.h>

/*
 * Boot program... arguments passed in r10 and r11 determine
 * whether boot stops to ask for system name and which device
 * boot comes from.
 */

/* Types in r10 specifying major device */
/* these are set by boot block, and should agree with stand/conf.c
 * and dev/conf.c
 */
char	devname[][2] = {
	'h','p',	/* 0 = hp */
	0,0,		/* 1 = ht */
	'u','p',	/* 2 = up */
	'h','k',	/* 3 = hk */
	0,0,		/* 4 = sw */
	0,0,		/* 5 = tm */
	0,0,		/* 6 = ts */
	'r','a',	/* 7 = ra */
	's', 'a',	/* 8 = sa (ra with 1k blocks) */
	'u', 't',	/* 9 = ut */
};

char line[100] = "xx(0,0)unix";

int	retry = 0;

main()
{
	register howto, devtype;	/* howto=r11, devtype=r10 */
	int io;

#ifdef lint
	howto = 0; devtype = 0;
#endif
	printf("\nBoot\n");
#ifdef JUSTASK
	howto = RB_ASKNAME|RB_SINGLE;
#else
	if ((howto&RB_ASKNAME)==0) {
		if (devtype>=0 && devtype<sizeof(devname)/2
		    && devname[devtype][0]) {
			line[0] = devname[devtype][0];
			line[1] = devname[devtype][1];
		} else
			howto = RB_SINGLE|RB_ASKNAME;
	}
#endif
	for (;;) {
		if (howto & RB_ASKNAME) {
			printf(": ");
			gets(line);
		} else
			printf(": %s\n", line);
		io = open(line, 0);
		if (io >= 0)
			copyunix(howto, io);
		if (++retry > 2)
			howto = RB_SINGLE|RB_ASKNAME;
	}
}

/*
 * read in a file and execute it
 * handles 0407 and 0410 files
 * anything else is assumed to be headerless image
 */

#define	BADMAGIC	0	/* anything not a valid magic number */
#define	HUGE	1000000L	/* big enough to reach EOF */

/*ARGSUSED*/
copyunix(howto, io)
	register howto, io;
{
	struct exec x;
	register int i;
	register char *addr;

	addr = (char *)0;
	if (read(io, (char *)&x, sizeof(x)) != sizeof(x)
	||  (x.a_magic != 0410 && x.a_magic != 0407)) {
		/*
		 * not a.out.  `exec' is part of text
		 */
		*(struct exec *)addr = x;
		x.a_magic = BADMAGIC;
		x.a_text = HUGE;
		x.a_data = 0;
		x.a_bss = 0;
		x.a_entry = 0;
		addr += sizeof(x);
	}
	printf("%d", x.a_text);
	if (read(io, addr, x.a_text) != x.a_text
	&&  x.a_magic != BADMAGIC)
		goto shread;
	addr += x.a_text;
	if (x.a_magic == 0410)
		while ((int)addr & CLOFSET)
			*addr++ = 0;
	printf("+%d", x.a_data);
	if (read(io, addr, x.a_data) != x.a_data)
		goto shread;
	addr += x.a_data;
	printf("+%d", x.a_bss);
	x.a_bss += 128*512;	/* slop */
	for (i = 0; i < x.a_bss; i++)
		*addr++ = 0;
	x.a_entry &= 0x7fffffff;
	printf(" start 0x%x\n", x.a_entry);
	close(io);
	(*((int (*)()) x.a_entry))();
	exit();
shread:
	_stop("Short read\n");
}
