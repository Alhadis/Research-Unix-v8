/*	uda.c	4.1	81/12/01	*/

/*
 * UDA50/RAxx disk device driver
 */

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/pte.h>
#include <sys/ubareg.h>
#include "saio.h"
#include "savax.h"

/*
 * Parameters for the communications area
 */
#define	NRSPL2	0
#define	NCMDL2	0
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)

#include <sys/udareg.h>
#include <sys/mscp.h>


u_short udastd[] = { 0772150 };

struct iob	cudbuf;

struct udadevice *udaddr = 0;

struct uda {
	struct udaca	uda_ca;
	struct mscp	uda_rsp;
	struct mscp	uda_cmd;
} uda;

struct uda *ud_ubaddr;			/* Unibus address of uda structure */

int uda_off[] = { 0, 20480, 0, -1, -1, -1, 49324, 131404 };

int udaref = 0;

struct mscp *udcmd();

udopen(io)
	register struct iob *io;
{
	register struct mscp *mp;
	int i;

	if (udaddr == 0)
		udaddr = (struct udadevice *)ubamem(io->i_unit, udastd[0]);
	if (ud_ubaddr == 0) {
		cudbuf.i_ma = (caddr_t)&uda;
		cudbuf.i_cc = sizeof(uda);
		ud_ubaddr = (struct uda *)ubasetup(&cudbuf, 2);
	}
	udaddr->udaip = 0;
	while ((udaddr->udasa & UDA_STEP1) == 0)
		;
	udaddr->udasa = UDA_ERR;
	while ((udaddr->udasa & UDA_STEP2) == 0)
		;
	udaddr->udasa = (short)&ud_ubaddr->uda_ca.ca_ringbase;
	while ((udaddr->udasa & UDA_STEP3) == 0)
		;
	udaddr->udasa = (short)(((int)&ud_ubaddr->uda_ca.ca_ringbase) >> 16);
	while ((udaddr->udasa & UDA_STEP4) == 0)
		;
	udaddr->udasa = UDA_GO;
	uda.uda_ca.ca_rspdsc[0] = (long)&ud_ubaddr->uda_rsp.mscp_cmdref;
	uda.uda_ca.ca_cmddsc[0] = (long)&ud_ubaddr->uda_cmd.mscp_cmdref;
	uda.uda_cmd.mscp_cntflgs = 0;
	if (udcmd(M_OP_STCON) == 0) {
		_stop("ra: open error, STCON");
		return;
	}
	uda.uda_cmd.mscp_unit = io->i_unit&7;
	if (udcmd(M_OP_ONLIN) == 0) {
		_stop("ra: open error, ONLIN");
		return;
	}
	if (io->i_boff < 0 || io->i_boff > 7 || uda_off[io->i_boff] == -1)
		_stop("ra: bad unit");
	io->i_boff = uda_off[io->i_boff];
	udaref++;
}

struct mscp *
udcmd(op)
	int op;
{
	struct mscp *mp;
	int i;

	uda.uda_cmd.mscp_opcode = op;
	uda.uda_rsp.mscp_header.uda_msglen = sizeof (struct mscp);
	uda.uda_cmd.mscp_header.uda_msglen = sizeof (struct mscp);
	uda.uda_ca.ca_rspdsc[0] |= UDA_OWN|UDA_INT;
	uda.uda_ca.ca_cmddsc[0] |= UDA_OWN|UDA_INT;
	i = udaddr->udaip;
	for (;;) {
		if (uda.uda_ca.ca_cmdint)
			uda.uda_ca.ca_cmdint = 0;
		if (uda.uda_ca.ca_rspint)
			break;
	}
	uda.uda_ca.ca_rspint = 0;
	mp = &uda.uda_rsp;
	if (mp->mscp_opcode != (op|M_OP_END) ||
	    (mp->mscp_status&M_ST_MASK) != M_ST_SUCC)
		return(0);
	return(mp);
}

udstrategy(io, func)
	register struct iob *io;
{
	register struct mscp *mp;
	int ubinfo;

	ubinfo = ubasetup(io, 1);
	mp = &uda.uda_cmd;
	mp->mscp_lbn = io->i_bn;
	mp->mscp_unit = io->i_unit&7;
	mp->mscp_bytecnt = io->i_cc;
	mp->mscp_buffer = (ubinfo & 0x3ffff) | (((ubinfo>>28)&0xf)<<24);
	if ((mp = udcmd(func == READ ? M_OP_READ : M_OP_WRITE)) == 0) {
		printf("ra: I/O error\n");
		ubafree(io, ubinfo);
		return(-1);
	}
	ubafree(io, ubinfo);
	return(io->i_cc);
}

udclose(io)
	register struct iob *io;
{

	if (--udaref <= 0) {
		udaref = 0;
		udaddr->udaip = 0;		/* reset */
	}
}
