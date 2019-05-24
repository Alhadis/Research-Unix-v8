#	@(#)uucp.mk	1.28
# this makefile is full of crap
# its more arcane options are not likely to work
# however, `make install' and `make all' (and makes of the individual
# a.out targets) will work well enough
#
# if you try to use make install, do it as uucp or root
# so the chmods and chowns will work.
# or just don't use make install.
#

#	/*  11/45, 11/70, and VAX version ('-i' has no effect on VAX)	*/
#	/* for 11/23, 11/34 (without separate I/D), IFLAG= */

#
# -DMANYSYS includes code to search for several Systems files; see sysopen.c
#

MANYSYS= -DMANYSYS

CFLAGS = -O ${MANYSYS}		# for normal case
CC=cc
IFLAG =
LDFLAGS= ${IFLAG}
#	NOTE - when UNETLIB is to be used, define UNET in parms.h
# add eio.o to PROTOCOLS below
# UNETLIB = -lunet
#	NOTE - when TCP is defined in parms.h
# add eio.o to PROTOCOLS below
#	NOTE - when DKLIB is to be used, define DATAKIT in parms.h
# add dio.o to PROTOCOLS below
# DKLIB = -ldk
# 	NOTE - some sites use -lbtl for compatibility
# BTLLIB = -lbtl 

# PROTOCOLS= dio.o	# for datakit
PROTOCOLS =	# if no datakit and no unet

LIBS= ${BTLLIB} ${DKLIB} ${UNETLIB}

# some versions of unix don't have strpbrk in libc
# STRPBRK = strpbrk.o
# STRPBRKSRC = strpbrk.c
# Use my getopt until the argument gobbling is fixed!

# use this on systems that don't have getopt() in libc
# GETOPT = getopt.o
# GETOPTSRC = getopt.c

OWNER=uucp
GRP=bin
BIN=$(ROOT)/usr/bin
INSDIR=$(ROOT)/usr/lib/uucp
UPATH=.
# The a.out files go to ODIR
ODIR=.

CLEAN=
LINTOP=
COMMANDS=uucp uux uuxqt uucico uusched
TOOLS=uuname uucheck uustat uucleanup
OFILES=utility.o cpmv.o expfile.o gename.o getpwinfo.o \
	ulockf.o xqt.o logent.o versys.o gnamef.o systat.o \
	$(GETOPT) sysopen.o
LFILES=utility.c cpmv.c expfile.c gename.c getpwinfo.c \
	ulockf.c xqt.c logent.c versys.c gnamef.c systat.c \
	$(GETOPTSRC) sysopen.c
OUUCP=uucpdefs.o uucp.o gwd.o permission.o getargs.o uucpname.o\
	versys.o gtcfile.o $(STRPBRK) chremdir.o 
LUUCP=uucpdefs.c uucp.c gwd.c permission.c getargs.c uucpname.c\
	versys.c gtcfile.c $(STRPBRKSRC) chremdir.c 
OUUX=uucpdefs.o uux.o gwd.o permission.o getargs.o getprm.o\
	uucpname.o versys.o gtcfile.o chremdir.o $(STRPBRK)
LUUX=uucpdefs.c uux.c gwd.c permission.c getargs.c getprm.c\
	uucpname.c versys.c gtcfile.c chremdir.c $(STRPBRKSRC)
OUUXQT=uucpdefs.o uuxqt.o mailst.o getprm.o gnamef.o uucpname.o \
	permission.o getargs.o gtcfile.o $(STRPBRK) \
	shio.o chremdir.o
LUUXQT=uucpdefs.c uuxqt.c mailst.c getprm.c gnamef.c uucpname.c \
	permission.c getargs.c gtcfile.c $(STRPBRKSRC) \
	shio.c chremdir.c
OUUCICO=uucpdefs.o cico.o conn.o callers.o cntrl.o pk0.o pk1.o \
	gio.o xio.o anlwrk.o permission.o getargs.o gnamef.o \
	gnxseq.o pkdefs.o imsg.o gtcfile.o \
	mailst.o uucpname.o line.o chremdir.o $(STRPBRK) $(PROTOCOLS)
LUUCICO=uucpdefs.c cico.c conn.c callers.c cntrl.c pk0.c pk1.c \
	gio.c xio.c anlwrk.c permission.c getargs.c gnamef.c \
	gnxseq.c pkdefs.c imsg.c gtcfile.c \
	mailst.c uucpname.c line.c chremdir.c $(STRPBRKSRC) eio.c dio.c
OUUNAME=uuname.o uucpname.o uucpdefs.o getpwinfo.o sysopen.o
LUUNAME=uuname.c uucpname.c uucpdefs.c getpwinfo.c sysopen.c
OUUSTAT=uustat.o gnamef.o expfile.o uucpdefs.o getpwinfo.o ulockf.o getargs.o \
	utility.o uucpname.o versys.o $(STRPBRK) sysopen.o 
LUUSTAT=uustat.c gnamef.c expfile.c uucpdefs.c getpwinfo.c ulockf.c getargs.c \
	utility.c uucpname.c versys.c $(STRPBRKSRC) sysopen.c
OUUSCHED=uusched.o gnamef.o expfile.o uucpdefs.o getpwinfo.o ulockf.o \
	systat.o getargs.o utility.o
LUUSCHED=uusched.c gnamef.c expfile.c uucpdefs.c getpwinfo.c ulockf.c \
	systat.c getargs.c utility.c
OUUCLEANUP=uucleanup.o gnamef.o expfile.o uucpdefs.o getpwinfo.o \
	uucpname.o ulockf.o getargs.o cpmv.o utility.o
LUUCLEANUP=uucleanup.c gnamef.c expfile.c uucpdefs.c getpwinfo.c \
	uucpname.c ulockf.c getargs.c cpmv.c utility.c
OUUGETTY=uugetty.o ulockf.o uucpdefs.o
LUUGETTY=uugetty.c ulockf.c uucpdefs.c

INIT=init

all:	$(INIT) $(COMMANDS) $(TOOLS) 

install:	all cp

cp:
	strip uuname uucp uux uustat
	cp uuname uucp uux uustat $(BIN)
	chown $(OWNER),$(GRP) $(BIN)/uuname $(BIN)/uucp $(BIN)/uux $(BIN)/uustat
	chmod 04775 $(BIN)/uuname $(BIN)/uucp $(BIN)/uux $(BIN)/uustat
	strip uucico uuxqt uusched uucheck uucleanup
	cp uucico uuxqt uusched uucheck uucleanup $(INSDIR)
	chown $(OWNER),$(GRP) $(INSDIR)/uucico $(INSDIR)/uuxqt $(INSDIR)/uusched $(INSDIR)/uucheck $(INSDIR)/uucleanup
# don't suid uuxqt, uucleanup
	chmod 04775 $(INSDIR)/uucico $(INSDIR)/uuxqt $(INSDIR)/uusched
# compatibility hacks.
	rm -f $(INSDIR)/uuci $(INSDIR)/uuco
	ln $(INSDIR)/uucico $(INSDIR)/uuci
	ln $(INSDIR)/uusched $(INSDIR)/uuco
# shell scripts
	cp kick debug uudemon.* uucrap remote.unknown $(INSDIR)
	chown $(OWNER),$(GRP) $(INSDIR)/kick $(INSDIR)/debug $(INSDIR)/uudemon.* $(INSDIR)/uucrap $(INSDIR)/remote.unknown
	chmod 0775 $(INSDIR)/kick $(INSDIR)/debug $(INSDIR)/uudemon.* $(INSDIR)/uucrap $(INSDIR)/remote.unknown
	cp uulog uupick uuto $(BIN)
	chown $(OWNER),$(GRP) $(BIN)/uulog $(BIN)/uupick $(BIN)/uuto
	chmod 0775  $(BIN)/uulog $(BIN)/uupick $(BIN)/uuto

clean:
	-rm -f *.o $(COMMANDS) $(TOOLS)

cmp:	all
	cmp uucp $(BIN)/uucp
	rm uucp
	cmp uux $(BIN)/uux
	rm uux
	cmp uuxqt $(INSDIR)/uuxqt
	rm uuxqt
	cmp uucico $(INSDIR)/uucico
	rm uucico
	cmp uuname $(BIN)/uuname
	rm uuname
	cmp uucheck $(INSDIR)/uucheck
	rm uucheck
	cmp uucleanup $(INSDIR)/uucleanup
	rm uucleanup
	cmp uustat $(BIN)/uustat
	rm uustat
	cmp uusched $(INSDIR)/uusched
	rm uusched

init:	anlwrk.o permission.o cpmv.o expfile.o gename.o \
	getargs.o getprm.o getpwinfo.o gnamef.o \
	gnxseq.o gwd.o imsg.o logent.o \
	mailst.o shio.o \
	systat.o ulockf.o uucpname.o versys.o xqt.o

uucp:	$(OUUCP) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUCP) $(OFILES) $(LIBS) \
		-o $(ODIR)/uucp

uux:	$(OUUX) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUX) $(OFILES) $(LIBS) \
		-o $(ODIR)/uux

uuxqt:	$(OUUXQT) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUXQT) $(OFILES) $(LIBS) \
		-o $(ODIR)/uuxqt

uucico:	$(OUUCICO) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUCICO) $(OFILES) $(LIBS) \
		-o $(ODIR)/uucico

uuname:	$(OUUNAME)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUNAME) $(LIBS) \
		-o $(ODIR)/uuname

uustat:	$(OUUSTAT) $(GETOPT)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUSTAT) $(GETOPT) $(LIBS) \
		-o $(ODIR)/uustat
 
uucleanup:	$(OUUCLEANUP) $(GETOPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUUCLEANUP) $(GETOPT) $(LIBS) \
		-o $(ODIR)/uucleanup
 
uusched:	$(OUUSCHED) $(GETOPT)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUSCHED) $(GETOPT) $(LIBS) \
		-o $(ODIR)/uusched
 
uucheck:	uucheck.o uucpname.o $(GETOPT) sysopen.o
	$(CC) $(CFLAGS)  $(LDFLAGS) uucheck.o uucpname.o $(GETOPT) sysopen.o $(LIBS) \
		-o $(ODIR)/uucheck
 
uugetty:	$(OUUGETTY)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUUGETTY) \
		-o $(ODIR)/uugetty
 
uucheck.o:	permission.c

utility.o permission.o cico.o conn.o callers.o cpmv.o\
	anlwrk.o cntrl.o expfile.o gename.o\
	getpwinfo.o gio.o xio.o gnamef.o gnxseq.o gwd.o imsg.o ioctl.o\
	logent.o mailst.o sdmail.o line.o shio.o\
	systat.o ulockf.o uucpdefs.o uucpname.o uuname.o\
	uucleanup.o uucheck.o uusched.o  uucp.o uustat.o dio.o \
	uux.o uuxqt.o versys.o xqt.o:	uucp.h parms.h

mkdirs:


#  lint procedures

lint:	lintuucp lintuucico lintuux lintuuxqt \
	lintuuname

lintuucp:
	lint $(LINTOP) $(LUUCP) $(LFILES)

lintuucico:
	lint $(LINTOP) $(LUUCICO) $(LFILES)

lintuux:
	lint $(LINTOP) $(LUUX) $(LFILES)

lintuuxqt:
	lint $(LINTOP) $(LUUXQT) $(LFILES)

lintuuname:
	lint $(LINTOP) $(LUUNAME)
