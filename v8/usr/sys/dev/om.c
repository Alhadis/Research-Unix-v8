/* ******************************************************************** */
/*									*/
/*			METHEUS Corporation				*/
/*									*/
/*		UNIX - DMA Interface Driver Source Code			*/
/*									*/
/*	This program and the subroutines implemented thereby are	*/
/*	proprietary information of Metheus Corporation and may not	*/
/*	be reproduced, or disclosed or released to, or used by any	*/
/*	other person without the express written consent of Metheus	*/
/*	Corporation.							*/
/*									*/
/*	(c) 1983 Metheus Corporation					*/
/*	All rights reserved						*/
/*									*/
/*	UNIX DR-11W DMA Interface Software by Ed Mills			*/
/*									*/
/*	Version: UNIX DMA Interface Driver low-level (OM) routines	*/
/*	Release: 1.0							*/
/*	Date: May 15, 1983						*/
/*									*/
/*									*/
/*									*/
/*	FUNCTIONAL DESCRIPTION:						*/
/*									*/
/*		OPEN:							*/
/*			omopen initializes the device and allows	*/
/*			IO to commence. Only one user may have the	*/
/*			device open at any given time.			*/
/*			omopen sets the read mode to no stall and	*/
/*			and the timeout period to 1 second. (see below)	*/
/*			omopen sets the write mode to no stall and	*/
/*			and the timeout period to 1 minute. (see below)	*/
/*									*/
/*		CLOSE:							*/
/*			omclose disallows further IO and makes the	*/
/*			device available for another user to open.	*/
/*									*/
/*		READ:							*/
/*			omread has two modes, stall and no stall.	*/
/*			In stall mode omread behaves as a standard	*/
/*			read. It stalls until the requested amount	*/
/*			of data has been read and then returns.		*/
/*			In no stall mode omread waits for the timeout	*/
/*			period (set via ioctl) to expire. If the	*/
/*			requested data is available before the end of	*/
/*			the timeout period, it is returned. If not,	*/
/*			then as much data as was available is returned.	*/
/*			In either case, the user level read returns the	*/
/*			actual number read.				*/
/*			Since the OMEGA 440 can only transmit words,	*/
/*			the count requested from omread must be even	*/
/*			or an IO error will be generated.		*/
/*									*/
/*		WRITE:							*/
/*			omwrite transmits the requested number of bytes	*/
/*			to the device. Like READ, WRITE has two modes,	*/
/*			stall and no stall which behave similarly to	*/
/*			omread's.					*/
/*			Since the OMEGA 440 can only transmit words,	*/
/*			the count sent to omwrite must be even or an	*/
/*			IO error will be generated.			*/
/*									*/
/*		IOCTL:							*/
/*			omioctl has 14 commands, OM_SETRSTALL,		*/
/*			OM_SETNORSTALL, OM_GETRSTALL,, OM_SETRTIMEOUT,	*/
/*			OM_GETRTIMEOUT, OM_SETWSTALL, OM_SETNOWSTALL,	*/
/*			OM_GETWSTALL, OM_SETWTIMEOUT, OM_GETWTIMEOUT,	*/
/*			OM_WRITEREADY, OM_READREADY, OM_BUSY,		*/
/*			and OM_RESET.					*/
/*									*/
/*		    OM_SETRSTALL:					*/
/*			Set omread to stall mode.			*/
/*									*/
/*		    OM_SETNORSTALL:					*/
/*			Set omread to no stall mode.			*/
/*									*/
/*		    OM_GETRSTALL:					*/
/*			Returns true if in read stall mode.		*/
/*									*/
/*		    OM_SETRTIMEOUT:					*/
/*			Sets the read stall mode timeout period in	*/
/*			tenths of a second.				*/
/*									*/
/*		    OM_GETRTIMEOUT:					*/
/*			Returns the read stall mode timeout period in	*/
/*			tenths of a second.				*/
/*									*/
/*		    OM_SETWSTALL:					*/
/*			Set omwrite to stall mode.			*/
/*									*/
/*		    OM_SETNOWSTALL:					*/
/*			Set omwrite to no stall mode.			*/
/*									*/
/*		    OM_GETWSTALL:					*/
/*			Returns true if in write stall mode.		*/
/*									*/
/*		    OM_SETWTIMEOUT:					*/
/*			Sets the write stall mode timeout period in	*/
/*			tenths of a second.				*/
/*									*/
/*		    OM_GETWTIMEOUT:					*/
/*			Returns the write stall mode timeout period in	*/
/*			tenths of a second.				*/
/*									*/
/*		    OM_WRITEREADY:					*/
/*			Returns a value of 1 if the device can accept	*/
/*			data, 0 otherwise. Internally this is the	*/
/*			DR11-W STAT A bit.				*/
/*									*/
/*		    OM_READREADY:					*/
/*			Returns a value of 1 if the device has data	*/
/*			to be read, 0 otherwise. Internally this is	*/
/*			the DR11-W STAT B bit.				*/
/*									*/
/*		    OM_BUSY:						*/
/*			Returns a value of 1 if the device is busy,	*/
/*			0 otherwise. Internally this is the DR11-W	*/
/*			STAT C bit.					*/
/*									*/
/*		    OM_RESET:						*/
/*			Resets the OMEGA to its power on condition	*/
/*			by asserting the CSR MAINTENANCE bit.		*/
/*									*/
/*									*/
/*	CAVEATS:							*/
/*									*/
/*		This driver is designed to allow multiple devices,	*/
/*			although it has only been tested with one.	*/
/*		This driver should also function with the DR11-B,	*/
/*			although this hasn't been tested either.	*/
/*									*/
/*	USER NOTES:							*/
/*									*/
/*		All IO is done with an even number of bytes. Hence,	*/
/*		when writing, a zero byte should be appended on the	*/
/*		end of odd length byte arrays. This must only be done	*/
/*		between commands, never within one. When reading, the	*/
/*		user must gaurentee that there is an even number number	*/
/*		of bytes to be read. i.e. send the read pixel command	*/
/*		twice.							*/
/*									*/
/*		Data pending to be read can prevent write operations.	*/
/*		Hopefully, one always knows when there is data to be	*/
/*		read. If not, one can read 2 bytes at a time until the	*/
/*		READREADY ioctl clears. CLEARINPUT_OM() in om-dmacros.h	*/
/*		does this.						*/
/*									*/
/*		If the OMEGA ever does hang, typing ^C's on the		*/
/*		terminal and THEN cycling the OMEGA's power seems	*/
/*		to fix things.						*/
/*									*/
/*									*/
/*	INSTALLATION:							*/
/*									*/
/*	The standard DR11-W parameters are,				*/
/*									*/
/*		Base address		0772410				*/
/*		CSR address		0772414				*/
/*		Interrupt vector	0124				*/
/*		Interrupt priority	5				*/
/*									*/
/*	The following line should be in the system configuration file.	*/
/* 
device		om0	at uba? csr 0172414		vector omintr
/*									*/
/*	The configure routine should reflect the new device.		*/
/*	(/usr/beaver/usr/sys/dev/conf.c on vlsi-vax.)			*/
/*									*/
/*	For vlsi-vax the following was added,

#include "om.h"
#if NOM > 0
int	omopen(),omclose(),omread(),omwrite(),omioctl();
#else
#define	omopen	nodev
#define	omclose	nodev
#define	omread	nodev
#define	omwrite	nodev
#define	omioctl	nodev
#endif

struct cdevsw	cdevsw[] =
{
	omopen,		omclose,	omread,		omwrite,
	omioctl,	nulldev,	nodev,		0,
	0,	
};
									*/
/* ******************************************************************** */
/*									*/
/*	Finally, om.h should be put in the configuration directory	*/
/*	to define NOM.							*/
/*									*/
/* ******************************************************************** */

#include "om.h"
#if NOM > 0

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/ubavar.h"
#include <sys/om-consts.h>

#define	OMPRI	(PZERO)

struct	omdevice {
	unsigned short	WC;	 /* 2410 Word count. */
	unsigned short	BAR;	 /* 2412 Bus address register. */
	unsigned short	CSR_EIR; /* 2414 Control and status register -
					 Error and information register. */
				 /* We never use the EIR. We should always
				    write CSR bit 15 as a zero. */
	unsigned short	IDR_ODR; /* 2416 Input data register -
					 Output data register. */
	};

/* DR11-W CSR register bits. */
#define	OM_ERROR	0100000
#define	OM_NEXMEM	0040000
#define	OM_ATTN		0020000
#define	OM_MAINT	0010000
#define	OM_STATA	0004000
#define	OM_STATB	0002000
#define	OM_STATC	0001000
#define	OM_CYCLE	0000400
#define	OM_READY	0000200
#define	OM_IENABLE	0000100
#define	OM_XBA		0000060
#define	OM_FUNCTION3	0000010
#define	OM_FUNCTION2	0000004
#define	OM_FUNCTION1	0000002
#define	OM_GO		0000001

/* Amounts to shift by to get the status bits from the CSR. */
#define	OM_STATA_SHIFT	11
#define	OM_STATB_SHIFT	10
#define	OM_STATC_SHIFT	9

/* DR11-W EIR register bits. OM_ERROR, OM_NEXMEM, and OM_ATTN also apply. */
#define	OM_MCREQ	0010000
#define	OM_ACLO		0004000
#define	OM_PARERR	0002000
#define	OM_BURSTDL	0001000
#define OM_NCBURST	0000400
#define	OM_EIRREG	0000001

/* The following are specific to the OMEGA 440 as interfaced to the DR11-W. */
#define	OM_READ		OM_FUNCTION1	/* Read from OMEGA. */
#define	OM_WRITE	0		/* Write to OMEGA. */
#define	OM_WAKEUP	OM_FUNCTION3	/* Have OMEGA assert ATTN line which
					   will cause an error, stop any pending
					   DMA, and cause an interrupt if
					   enabled. */
#define	OM_WRITE_SHIFT	OM_STATA_SHIFT	/* Ready to write bit. */
#define	OM_READ_SHIFT	OM_STATB_SHIFT	/* Ready to read bit. */
#define	OM_BUSY_SHIFT	OM_STATC_SHIFT	/* Busy bit. */

struct om_softc {
	short	sc_state;	/* Current state of the device. */
	short	sc_operation;	/* Current operation (read or write). */
	unsigned int
		sc_rtimoticks,	/* Number of ticks before timing out
				   on a no stall read. */
		sc_wtimoticks,	/* Number of ticks before timing out
				   on a no stall write. */
		sc_currenttimo;	/* The number of the current timeout call
				   to omrwtimo. */
	int	sc_ubinfo;	/* UNIBUS information. */
	int	sc_ubadd;	/* physical unibus address for cursor	*/
unsigned char	sc_curbuf[512];	/* buffer for cursor dma commands	*/
} om_softc[NOM];


/* sc_state bits */
#define	OMSC_OPEN	(1 << 0)	/* The device is open. */
#define	OMSC_BUSY	(1 << 1)	/* The device is busy. */
#define	OMSC_NORSTALL	(1 << 2)	/* The device is set to use
					   no stall mode for reads. */
#define	OMSC_NOWSTALL	(1 << 3)	/* The device is set to use
					   no stall mode for writes. */
#define	OMSC_TIMEDOUT	(1 << 4)	/* The device timed out on a
					   stall mode read or write. */

struct	uba_device *omdinfo[NOM];

#define	OMUNIT(dev)	(minor(dev))

struct	buf rombuf[NOM];

int	omprobe(), omattach(), omstrategy(), omrwtimo();
unsigned minomph();

u_short	omstd[] = { 0772410, 0772430, 0 };
struct	uba_driver omdriver = {
	omprobe,	/* Address of probe routine to cause an interrupt. */
	0,		/* ? */
	omattach,	/* Address of attach routine to set IO address. */
	0,		/* ? */
	omstd,		/* Address of the device (omdevice) on the UNIBUS. */
	"om",		/* Name of the device. */
	omdinfo,	/* UBA device information for the device. */
	0, 0
	};


/* OMPROBE */
/* Omprobe generates an interrupt when called by asserting the OMEGA attention
   line with interrupts enabled. */

omprobe(reg)
	caddr_t reg;
{
	register int br, cvec;		/* value-result */
	register struct omdevice *omaddr = (struct omdevice *)(reg-04);

#ifdef lint
	br = 0; cvec = br; br = cvec;
#endif
	/* Stop any DMA and force an interrupt. */
	omaddr->CSR_EIR = (OM_ATTN | OM_WAKEUP | OM_IENABLE);
	DELAY(100);
	omaddr->CSR_EIR = OM_IENABLE;
	/* kludge */
	br = 0x15;
	switch ((int)omaddr & 070)
	{
	case 010:
		cvec = 0124;
		break;
	case 030:
		cvec = 0134;
		break;
	}
	return (sizeof (struct omdevice));
}


/* OMATTACH */
/* Attach to and initialize the device. */

omattach(ui)	struct uba_device *ui;
{
 register struct omdevice *omaddr;
 register struct om_softc *omsp;

	/* Move the address back from the CSR to the begining of the device. */
	ui->ui_addr -= 04;
	ui->ui_physaddr -= 04;
	omaddr = (struct omdevice *)ui->ui_addr;

	/* Initialize the DR11-W by setting then clearing MAINT in the CSR
	   with CYCLE and GO  clear. */
	omaddr->CSR_EIR = 0;
	omaddr->CSR_EIR = OM_MAINT;
	DELAY(100);
	omaddr->CSR_EIR = 0;

	omaddr->CSR_EIR = OM_IENABLE;	/* Enable interrupts. */

	/* Set the current timeout to zero. */
	/* What I want here is for OMUNIT(ui->ui_unit) to give the minor
	   device number of each device as we attach to it. I think that's
	   what it does. When there is only one device, I know that's what
	   it does. I check range just to make sure things never get out
	   of hand. Since all we need from sc_currenttimo is that it be
	   distinct each time we increment it, we don't really care where
	   it starts. */
	omsp = &om_softc[OMUNIT(ui->ui_unit)];
	if ((OMUNIT(ui->ui_unit) >= 0) && (OMUNIT(ui->ui_unit) < NOM))
		omsp->sc_currenttimo = 0;

	/* allocate a physical unibus dma buffer for cursor tracking */
	if(omsp->sc_ubadd == 0)
	  { omsp->sc_ubadd = uballoc(0,	 /* which unibus adapter */
				(caddr_t)omsp->sc_curbuf,
				sizeof(omsp->sc_curbuf), 0);
	    if(omsp->sc_ubadd == 0)
		panic("UB AD 0 in omattach"); 
	  }
}

/* OMOPEN */
/* Open the device. */

omopen(dev)
	dev_t dev;
{
	register struct om_softc *sc;
	register struct uba_device *ui;
	register struct omdevice *omaddr;

	if ((OMUNIT(dev) >= NOM) ||	/* No such device. */
	    ((ui = omdinfo[OMUNIT(dev)]) == 0)) { /* Device not there. */
		u.u_error = ENXIO;
		return;
		}

	if ((ui->ui_alive) == 0) { /* Dead. */
		u.u_error = EIO;
		return;
		}

	if (((sc = &om_softc[OMUNIT(dev)])->sc_state)&OMSC_OPEN) {
		u.u_error = EBUSY;	/* Already open. */
		return;
		}

	/* Set only the open bit in the state. */
	(sc->sc_state) = OMSC_OPEN;

	/* Set the read no stall timeout to 1 second. */
	(sc->sc_rtimoticks) = hz;

	/* Set the write no stall timeout to 1 minute. */
	(sc->sc_wtimoticks) = hz*60;

	/* Start the self-kicker. */
	omtimo(dev);

	/* Stop any DMA without causing an interrupt. (No OM_IENABLE bit.) */
	omaddr = (struct omdevice *)ui->ui_addr;
	omaddr->CSR_EIR = (OM_WAKEUP | OM_ATTN);
	DELAY(100);

/* ************************************************************************ */
/*
/*	The following commented piece of code will cause the OMEGA to do a
/*	hardware reset each time it is opened. This gaurentees the device to
/*	be in a known state, but doesn't allow a program to manipulate an
/*	image produced by a previous program since it clears the screen.
/*
/* ************************************************************************ */
/*
/*	/* Initialize the DR11-W by setting then clearing MAINT in the CSR
/*	   with CYCLE and GO  clear. */
/*	omaddr->CSR_EIR = 0;
/*	omaddr->CSR_EIR = OM_MAINT;
/*	DELAY(100);
/*	omaddr->CSR_EIR = 0;
/*
/*	/* Wait for the omega to become ready. */
/*	while((omaddr->CSR_EIR)&OM_STATC)
/*		sleep((caddr_t)sc, OMPRI);
/*
/* ************************************************************************ */

	/* Enable interrupts. */
	omaddr->CSR_EIR = OM_IENABLE;


	/* Make sure the device is really there and OK. */
/* forget this! */
/*	if (((omaddr->CSR_EIR) >> OM_BUSY_SHIFT) & 1) {
/*		u.u_error = EIO;
/*		sc->sc_state = 0;
/*		return;
/*		}
*/
}


/* OMCLOSE */
/* Close the device. */

omclose(dev)
	dev_t dev;
{
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];

	/* Set the state blank. */
	sc->sc_state = 0;
}


/* OMREAD */
/* Reads characters from the device. If in stall mode, the read will be the
   standard read which will not return until the requested number of characters
   have been read. If in no stall mode, then the read will return after an
   interval set in omioctl and report how many characters it read.
   The count must be even since the DMA is by words. */

omread(dev)
	dev_t dev;
{
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];
	register struct omdevice *omaddr =
	    (struct omdevice *)omdinfo[OMUNIT(dev)]->ui_addr;
	register int spl;

	/* Make sure that the count is even. */
	if (u.u_count & 1) {
		u.u_error = EINVAL;
		return;
		}

	/* Set the DR11-W to read. */
	sc->sc_operation = OM_READ;

	/* The following is to make up for something physio should really do
	   for us. If bp->b_resid has been left non-zero by the last call,
	   (ie an error), and the present call has a count of zero then the
	   old residual will be returned as the count. To correct this, we just
	   set the residual to zero before we start. */
	rombuf[OMUNIT(dev)].b_resid = 0;

	/* Are we in no stall or stall mode? */
	if ((sc->sc_state) & OMSC_NORSTALL) {

		/* We're in no stall mode. */

		/* Start the timer running. */
		/* Don't let anything stop us once the timer's running. */
		spl = spl6();
		timeout(omrwtimo, (caddr_t)
				( ((sc->sc_currenttimo)<<8) | OMUNIT(dev) ),
							(sc->sc_rtimoticks));

		/* Perform the read. */
		physio(omstrategy, &rombuf[OMUNIT(dev)], dev, B_READ, minomph);
		if (u.u_error) (void) splx(spl);

		/* Update the current timeout number. */
		(sc->sc_currenttimo)++;

		/* Did we timeout? */
		if ((sc->sc_state) & OMSC_TIMEDOUT) {

			/* Clear the timeout. */
			(sc->sc_state) &= (~OMSC_TIMEDOUT);

			/* Patch up the error. */

			/* We made the error ourself, ignore it. */
			u.u_error = 0;
			}
		}

	    else {

		/* We're in stall mode. */

		/* Perform the read. */
		physio(omstrategy, &rombuf[OMUNIT(dev)], dev, B_READ, minomph);
		}
}


/* OMWRITE */
/* omwrite writes the passed data to the device using DMA.
   The count must be even since the DMA is by words. */
/* Timeouts are handled as in omread. */

omwrite(dev)
	dev_t dev;
{
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];
	register int spl;

	/* Make sure that the count is even. */
	if (u.u_count & 1) {
		u.u_error = EINVAL;
		return;
		}

	/* This call is to write around a system bug--on a B_WRITE call to
	   physio, the system checks to see if the user has read access to the
	   requested block.  Then it locks necessary pages in virtual memory.
	   It checks for write access in the course of this, and, if the user
	   does not have it, panics ("vslock").  So, if the user sends, say, an
	   address in his code (0 is an example), the system crashes.  So I
	   start by checking for write access. */
	if (useracc(u.u_base,u.u_count,B_WRITE) == NULL) {
		u.u_error = EFAULT;
		return;
		}

	/* Set the DR11-W to write. */
	sc->sc_operation = OM_WRITE;

	/* The following is to make up for something physio should really do
	   for us. If bp->b_resid has been left non-zero by the last call,
	   (ie an error), and the present call has a count of zero then the
	   old residual will be returned as the count. To correct this, we just
	   set the residual to zero before we start. */
	rombuf[OMUNIT(dev)].b_resid = 0;

	/* Are we in no stall or stall mode? */
	if ((sc->sc_state) & OMSC_NOWSTALL) {

		/* We're in no stall mode. */

		/* Start the timer running. */
		/* Don't let anything stop us once the timer's running. */
		spl = spl6();
		timeout(omrwtimo, (caddr_t)
				( ((sc->sc_currenttimo)<<8) | OMUNIT(dev) ),
							(sc->sc_wtimoticks));

		/* Perform the write. */
		physio(omstrategy, &rombuf[OMUNIT(dev)], dev,
							B_WRITE, minomph);
		if (u.u_error) (void) splx(spl);

		/* Update the current timeout number. */
		(sc->sc_currenttimo)++;

		/* Did we timeout? */
		if ((sc->sc_state) & OMSC_TIMEDOUT) {

			/* Clear the timeout. */
			(sc->sc_state) &= (~OMSC_TIMEDOUT);

			/* Patch up the error. */

			/* We made the error ourself, ignore it. */
			u.u_error = 0;
			}
		}

	    else {

		/* We're in stall mode. */

		/* Perform the write. */
		physio(omstrategy, &rombuf[OMUNIT(dev)], dev,
							B_WRITE, minomph);
		}
}


/* OMIOCTL */
/* IO control function. */

omioctl(dev, cmd, addr, flag)
	dev_t dev;
	int cmd;
	register caddr_t addr;
	int flag;

{
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];
	register struct omdevice *omaddr =
	    (struct omdevice *)omdinfo[OMUNIT(dev)]->ui_addr;
	int temp;

	switch(cmd) {

	case OM_SETRSTALL:

		/* Set read stall mode. */
		(sc->sc_state) &= (~OMSC_NORSTALL);
		break;

	case OM_SETNORSTALL:

		/* Set read stall mode. */
		(sc->sc_state) |= OMSC_NORSTALL;
		break;

	case OM_GETRSTALL:

		/* Returns true if in read stall mode. */
		temp = (!((sc->sc_state)&OMSC_NORSTALL));
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_SETRTIMEOUT:

		/* Set the number of ticks before a no stall read times out.
		   The argument is given in tenths of a second. */
		if (copyin(addr, (caddr_t)&temp, sizeof(temp))) {
			u.u_error = EFAULT;
			break;
			}
		if (temp < 1) {
			u.u_error = EINVAL;
			temp = 1;
			}
		(sc->sc_rtimoticks) = (temp * hz )/10;
		break;

	case OM_GETRTIMEOUT:

		/* Returns the number of tenths of seconds before
		   a no stall read times out. */
		/* The argument is given in tenths of a second. */
		temp = ((sc->sc_rtimoticks)*10)/hz;
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_SETWSTALL:

		/* Set write stall mode. */
		(sc->sc_state) &= (~OMSC_NOWSTALL);
		break;

	case OM_SETNOWSTALL:

		/* Set write stall mode. */
		(sc->sc_state) |= OMSC_NOWSTALL;
		break;

	case OM_GETWSTALL:

		/* Returns true if in write stall mode. */
		temp = (!((sc->sc_state)&OMSC_NOWSTALL));
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_SETWTIMEOUT:

		/* Set the number of ticks before a no stall write times out.
		   The argument is given in tenths of a second. */
		if (copyin(addr, (caddr_t)&temp, sizeof(temp))) {
			u.u_error = EFAULT;
			break;
			}
		if (temp < 1) {
			u.u_error = EINVAL;
			temp = 1;
			}
		(sc->sc_wtimoticks) = (temp * hz )/10;
		break;

	case OM_GETWTIMEOUT:

		/* Returns the number of tenths of seconds before
		   a no stall write times out. */
		/* The argument is given in tenths of a second. */
		temp = ((sc->sc_wtimoticks)*10)/hz;
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_WRITEREADY:

		/* Returns a value of 1 if the device can accept
		   data, 0 otherwise. Internally this is the
		   DR11-W STAT A bit. */
		temp = (((omaddr->CSR_EIR) >> OM_WRITE_SHIFT) & 1);
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_READREADY:

		/* Returns a value of 1 if the device has data
		   to be read, 0 otherwise. Internally this is
		   the DR11-W STAT B bit. */
		temp = (((omaddr->CSR_EIR) >> OM_READ_SHIFT) & 1);
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_BUSY:

		/* Returns a value of 1 if the device is busy,
		   0 otherwise. Internally this is the DR11-W
		   STAT C bit. */
		temp = (((omaddr->CSR_EIR) >> OM_BUSY_SHIFT) & 1);
		if (copyout((caddr_t)&temp, addr, sizeof(temp))) {
			u.u_error = EFAULT;
			}
		break;

	case OM_RESET:

		/* Resets the OMEGA to its power on condition
		   by asserting the CSR MAINTENANCE bit. */

		/* Initialize the DR11-W by setting then clearing MAINT
		   in the CSR with CYCLE and GO clear. */
		omaddr->CSR_EIR = 0;
		omaddr->CSR_EIR = OM_MAINT;
		DELAY(100);
		omaddr->CSR_EIR = 0;

		/* Wait for the omega to become ready. */
/* forget this! */
/*		while((omaddr->CSR_EIR)&OM_STATC)
/*			tsleep((caddr_t)sc, OMPRI, 30);
*/
		break;

	default:

		/* Flag the error. */
		u.u_error = EINVAL;
		break;

	}

	return;
}


/* OMSTRATEGY */
/* Routine to determine the DMA strategy for physio. */

omstrategy(bp)
	register struct buf *bp;
{
	register int e;
	register int spl;
	register struct om_softc *sc = &om_softc[OMUNIT(bp->b_dev)];
	register struct uba_device *ui = omdinfo[OMUNIT(bp->b_dev)];
	register struct omdevice *omaddr = (struct omdevice *)ui->ui_addr;

	spl = spl6();
	while (sc->sc_state & OMSC_BUSY)
		tsleep((caddr_t)sc, OMPRI, 30);
	sc->sc_state |= OMSC_BUSY;
	sc->sc_ubinfo = ubasetup(ui->ui_ubanum, bp, UBA_NEEDBDP);
	omstart(bp->b_dev, bp->b_bcount);
	e = omwait(bp->b_dev);
	(bp->b_resid) = u.u_count - (bp->b_bcount) +
						(((-(omaddr->WC))&0xFFFF)<<1);
	(void) splx(spl);
	ubarelse(ui->ui_ubanum, &sc->sc_ubinfo);
	sc->sc_state &= ~OMSC_BUSY;
	iodone(bp);
	if (e)
		/* Cause physio to terminate and
		   geterror to give an EIO error. */
		(bp->b_flags) |= B_ERROR;
	wakeup((caddr_t)sc);
}


/* MINOMPH */
/* Set the maximum physical transfer size. */

unsigned
minomph(bp)
	struct buf *bp;
{

	if (bp->b_bcount > OM_BLOCKSIZE)
		bp->b_bcount = OM_BLOCKSIZE;
}


/* OMSTART */
/* Start a DMA IO transfer. */

omstart(dev, count)
	dev_t dev;
	long count;
{
	register int spl;
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];
	register struct omdevice *omaddr =
	    (struct omdevice *)omdinfo[OMUNIT(dev)]->ui_addr;
	int i;

	spl = spl7();	/* don't interrupt me with a cursor */
	for (i = 100; (omaddr->CSR_EIR & OM_READY == 0) && i; --i)
		;	/* wait in case cursor dma active */
	if (i == 0)
	{
		(void) splx(spl);
		printf("omstart: device busy\n");
		u.u_error = EIO;
		return;
	}

	/* Set the DMA buffer address. */
	(omaddr->BAR) = (sc->sc_ubinfo);

	/* Set the word count to the 2's complement
	   of the count in words. */
	(omaddr->WC) = (-(count>>1));

	/* Start the transfer with interrupts enabled. */
	(omaddr->CSR_EIR) = (OM_IENABLE | (sc->sc_operation) |
	     ((sc->sc_ubinfo >> 12) & OM_XBA));
	(omaddr->CSR_EIR) = (OM_IENABLE | (sc->sc_operation) |
	     ((sc->sc_ubinfo >> 12) & OM_XBA)| OM_GO);

	(void) splx(spl);
	return;
}


/* OMRWTIMO */
/* Routine to deal with no stall read/write timeouts. If the read/write
   completed by itself, we just return. If not, we mark a timeout and
   cause a DMA error and interrupt. */

omrwtimo(timeoutinfo)
	register unsigned int timeoutinfo;
	/* The low 8 bits of timeoutinfo are the minor device number.
	   The remaining higher bits are the current timeout number. */
{
	register int minordev = OMUNIT(timeoutinfo);
	register struct om_softc *sc = &om_softc[minordev];
	register struct omdevice *omaddr =
		(struct omdevice *)omdinfo[minordev]->ui_addr;

	/* If this is not the timeout that omread/omwrite is waiting for
	   then we should just go away. */
	if ((timeoutinfo&(~0377)) != ((sc->sc_currenttimo)<<8)) return;

	/* Mark that the device timed out. */
	(sc->sc_state) |= OMSC_TIMEDOUT;

	/* Send an OM_WAKEUP signal to halt any pending DMA and generate
	   an interrupt so that omwait will see the error and tell everyone
	   the transfer is done. */
	omaddr->CSR_EIR = (OM_ATTN | OM_WAKEUP | OM_IENABLE);
}


/* OMWAIT */
/* Wait for the DR11-W to come ready.
   Return a non-zero value if there is an error. */

omwait(dev)
	dev_t dev;
{
	register struct omdevice *omaddr =
	    (struct omdevice *)omdinfo[OMUNIT(dev)]->ui_addr;
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];
	register int e;

	while (!((e = (omaddr->CSR_EIR)) & OM_READY)) {
		tsleep((caddr_t)sc, OMPRI, 30);
		}

	/* Reset any error conditions. */
	(omaddr->CSR_EIR) = OM_IENABLE;

	return (e & OM_ERROR);
}


/* OMINTR */
/* Wakup any process waiting for an interrupt. */

omintr(dev)
	dev_t dev;
{
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];

	wakeup((caddr_t)sc);
}


/* OMTIMO */
/* Kick the driver every second. */

omtimo(dev)
	dev_t dev;
{
	register struct om_softc *sc = &om_softc[OMUNIT(dev)];

	if (sc->sc_state&OMSC_OPEN)
		timeout(omtimo, (caddr_t)dev, hz);
	omintr(dev);
}

omcur(dev, x, y)	/* position cursor - called from bpd driver */
dev_t dev;
{
        register struct uba_device *ui = omdinfo[minor(dev)];
        register struct omdevice *omp = (struct omdevice *)(ui->ui_addr);
        register struct om_softc *oms = &om_softc[minor(dev)];
        register i, j;
	int spl;

	spl = spl6();
        if( (oms->sc_state & OMSC_OPEN == 0) ||
	    (oms->sc_state & OMSC_BUSY)      ||
	    (omp->CSR_EIR & OM_READY == 0) )
		return;
        x *= 1280; x /= 1024;	/* scale x coordinate	*/
	y = 1024 - y;		/* negate y coordinate	*/
	/* setup cursor command */
	oms->sc_curbuf[0] = 0x52;		/* movp1 opcode	*/
	oms->sc_curbuf[1] = x & 0xFF;
	oms->sc_curbuf[2] = (x >> 8) & 0x07;
	oms->sc_curbuf[3] = y & 0xFF;
	oms->sc_curbuf[4] = (y >> 8) & 0x07;
	oms->sc_curbuf[5] = 0x71;		/* display cursor opcode */

	omp->BAR = oms->sc_ubadd;		/* setup dma	*/
	omp->WC = -3;	
	omp->CSR_EIR = OM_WRITE|(OM_XBA & (oms->sc_ubadd >> 12));
	omp->CSR_EIR = OM_WRITE|OM_GO|(OM_XBA & (oms->sc_ubadd >> 12));
	splx(spl);
}
