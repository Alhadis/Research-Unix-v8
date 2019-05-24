/*	conf.c	4.9	81/12/01	*/

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/pte.h>
#include <sys/mbareg.h>
#include "saio.h"

devread(io)
	register struct iob *io;
{

	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io, READ) );
}

devwrite(io)
	register struct iob *io;
{

	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE) );
}

devopen(io)
	register struct iob *io;
{

	(*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
	register struct iob *io;
{

	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

nullsys()
{

	;
}

int	nullsys();
#ifdef giantone
int	hpstrategy(), hpopen();
int	htstrategy(), htopen(), htclose();
int	mtstrategy(), mtopen(), mtclose();
int	rkstrategy(), rkopen();
int	tmstrategy(), tmopen(), tmclose();
int	tsstrategy(), tsopen(), tsclose();
int	utstrategy(), utopen(), utclose();
#else
int	hpstrategy(), hpopen();
#define htstrategy nullsys
#define htopen nullsys
#define htclose nullsys
#define mtstrategy nullsys
#define mtopen nullsys
#define mtclose nullsys
#define rkstrategy nullsys
#define rkopen nullsys
#define tmstrategy nullsys
#define tmopen nullsys
#define tmclose nullsys
#define tsstrategy nullsys
#define tsopen nullsys
#define tsclose nullsys
#define utstrategy nullsys
#define utopen nullsys
#define utclose nullsys
#endif
int	udstrategy(), udopen(), udclose();
int	upstrategy(), upopen();

struct devsw devsw[] = {
	"hp",	hpstrategy,	hpopen,		nullsys,	SMALL,
	"ht",	htstrategy,	htopen,		htclose,	SMALL,
	"up",	upstrategy,	upopen,		nullsys,	SMALL,
	"tm",	tmstrategy,	tmopen,		tmclose,	SMALL,
	"hk",	rkstrategy,	rkopen,		nullsys,	SMALL,
	"ts",	tsstrategy,	tsopen,		tsclose,	SMALL,
	"mt",	mtstrategy,	mtopen,		mtclose,	SMALL,
	"ra",	udstrategy,	udopen,		udclose,	BIG,
	"sa",	udstrategy,	udopen,		udclose,	SMALL,
	"ut",	utstrategy,	utopen,		utclose,	SMALL,
	0,0,0,0,0,
};
