/*
 * access the files:
 * read or write data from some address in some space
 */

#include "defs.h"
#include "regs.h"
#include <sys/param.h>

/*
 * routines to get/put various types
 */

TLONG
lget(addr, space)
ADDR addr;
int space;
{
	TLONG x;

	if ((space & SPTYPE) == NOSP)
		return (wtol(dot));
	if (fget(addr, space, (char *)&x, sizeof(x)) == 0)
		return (0);
	return (x);
}

TSHORT
sget(addr, space)
ADDR addr;
int space;
{
	TSHORT x;

	if ((space & SPTYPE) == NOSP)
		return (wtos(dot));
	if (fget(addr, space, (char *)&x, sizeof(x)) == 0)
		return (0);
	return (x);
}

TCHAR
cget(addr, space)
ADDR addr;
int space;
{
	TCHAR x;

	if ((space & SPTYPE) == NOSP)
		return (wtoc(dot));
	if (fget(addr, space, (char *)&x, sizeof(x)) == 0)
		return (0);
	return (x);
}

TADDR
aget(addr, space)
ADDR addr;
int space;
{
	TADDR x;

	if ((space & SPTYPE) == NOSP)
		return (wtoa(dot));
	if (fget(addr, space, (char *)&x, sizeof(x)) == 0)
		return (0);
	return (x);
}

lput(addr, space, v)
ADDR addr;
int space;
TLONG v;
{

	return (fput(addr, space, (char *)&v, sizeof(v)));
}

sput(addr, space, v)
ADDR addr;
int space;
TSHORT v;
{

	return (fput(addr, space, (char *)&v, sizeof(v)));
}

cput(addr, space, v)
ADDR addr;
int space;
TCHAR v;
{

	return (fput(addr, space, (char *)&v, sizeof(v)));
}

aput(addr, space, v)
ADDR addr;
int space;
TADDR v;
{

	return (fput(addr, space, (char *)&v, sizeof(v)));
}

/*
 * the real io code
 */

int
fget(addr, space, buf, size)
ADDR addr;
register int space;
char *buf;
int size;
{
	int fd;
	off_t off;
	ADDR oa;
	ADDR regaddr();
	off_t lseek();

	if ((space & SPTYPE) == NOSP) {
		memset(buf, 0, size);
		memcpy(buf, (char *)&dot, size < sizeof(dot) ? sizeof(dot) : size);
		return (1);
	}
	if (space & SYMF)
		fd = fsym;
	else
		fd = fcor;
	oa = addr;
	if ((space & SPTYPE) == REGSP) {
		if (space & SYMF) {
			errflg = "registers in corefile only";
			return (0);
		}
		if ((addr = regaddr(addr)) == 0)
			return (intrget(oa, buf, size));
		space &=~ SPTYPE;
		space |= DATASP;
	}
	if (reloc(addr, space, &off) == 0)
		return (0);
	if (lseek(fd, off, 0) == -1
	||  read(fd, buf, size) != size) {
		if ((space & SPTYPE) == INSTSP)
			errflg = "can't read text";
		else
			errflg = "can't read data";
		return (0);
	}
	return (1);
}

int
fput(addr, space, buf, size)
ADDR addr;
register int space;
char *buf;
int size;
{
	int fd;
	off_t off;
	ADDR oa;
	ADDR regaddr();
	off_t lseek();

	if ((space & SPTYPE) == NOSP)
		return (0);
	if (wtflag == 0
	&&  !((space & SYMF) == 0 && pid))
		error("not in write mode");
	if (space & SYMF)
		fd = fsym;
	else
		fd = fcor;
	oa = addr;
	if ((space & SPTYPE) == REGSP) {
		if (space & SYMF) {
			errflg = "registers in corefile only";
			return (0);
		}
		if ((addr = regaddr(addr)) == 0)
			return (intrput(oa, buf, size));
		space &=~ SPTYPE;
		space |= DATASP;
	}
	if (reloc(addr, space, &off) == 0)
		return (0);
	if (lseek(fd, off, 0) == -1
	||  write(fd, buf, size) != size) {
		if ((space & SPTYPE) == INSTSP)
			errflg = "can't write text";
		else
			errflg = "can't write data";
		return (0);
	}
	return (1);
}

/*
 * register address hacks
 * if the register allegedly has an address (e.g. we're knee deep in
 * stack frames), return that
 * otherwise do fake io to our internal copies of registers
 * awful
 */

static ADDR
regaddr(reg)
register ADDR reg;
{
	extern ADDR raddr[];

	reg /= sizeof(TREG);
	if (MINREG <= reg && reg <= MAXREG)
		return (raddr[reg - MINREG]);
	return (0);
}

static int
intrget(reg, buf, size)
ADDR reg;
char *buf;
{
	register char *p, *q;
	register int n;
	register int rnum;
	TREG r;
	extern int roffs[];

	for (p = buf, n = size; n > 0; n--)
		*p++ = 0;
	rnum = reg / sizeof(TREG);
	if (rnum < MINREG)
		return (0);
	p = buf;
	while (rnum <= MAXREG && size > 0) {
		r = rget(roffs[rnum - MINREG]);
		for (q = (char *)&r, n = sizeof(TREG); n > 0; n--)
			*p++ = *q++;
		size -= sizeof(TREG);
		rnum++;
	}
	return (size <= 0);
}

static int
intrput(reg, buf, size)
ADDR reg;
char *buf;
{
	register char *p, *q;
	register int n;
	register int rnum;
	TREG r;
	extern int roffs[];

	rnum = reg / sizeof(TREG);
	if (rnum < MINREG)
		return (0);
	p = buf;
	while (rnum <= MAXREG && size > 0) {
		r = rget(roffs[rnum - MINREG]);
		for (q = (char *)&r, n = sizeof(TREG); n > 0 && size > 0; n--, size--)
			*q++ = *p++;
		rput(roffs[rnum - MINREG], r);
		rnum++;
	}
	return (size <= 0);
}

/*
 * turn address to file offset
 * returns nonzero if ok
 */


int
reloc(addr, space, offp)
ADDR addr;
register int space;
off_t *offp;
{
	register struct map *mp;

	for (mp = (space & SYMF) ? symmap : cormap; mp->flag & MPINUSE; mp++)
		if ((space & SPTYPE) == (mp->sp & SPTYPE)
		&&  mp->b <= addr && addr < mp->e) {
			addr += mp->f - mp->b;
			if ((space & (SYMF|RAWADDR)) == 0)
				if (kmap(&addr, space) == 0)
					return (0);
			*offp = addr;
			return (1);
		}
	/*
	 * if we wanted instruction space and didn't find it,
	 * try data space now
	 */
	if ((space & SPTYPE) == INSTSP) {
		space &=~ SPTYPE;
		space |= DATASP;
		for (mp = (space & SYMF) ? symmap : cormap; mp->flag & MPINUSE; mp++)
			if ((space & SPTYPE) == (mp->sp & SPTYPE)
			&&  mp->b <= addr && addr < mp->e) {
				addr += mp->f - mp->b;
				if ((space & (SYMF|RAWADDR)) == 0)
					if (kmap(&addr, space) == 0)
						return (0);
				*offp = addr;
				return (1);
			}
	}
	if (space & SYMF)
		errflg = "text address not found";
	else
		errflg = "data address not found";
	return (0);
}
