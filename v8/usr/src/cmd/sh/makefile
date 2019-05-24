#	@(#)sh.mk	1.5

TESTDIR = .
FRC =
ROOT=
INSDIR =
CFLAGS = -gd2
LDFLAGS =

OFILES = setbrk.o blok.o stak.o cmd.o fault.o main.o word.o string.o\
name.o args.o xec.o service.o error.o io.o print.o macro.o expand.o\
ctype.o msg.o defs.o pathserv.o func.o spname.o

all: sh

sh: $(SFILES) $(OFILES)
	$(CC) $(LDFLAGS) $(SFILES) $(OFILES) -o $(TESTDIR)/sh

$(OFILES):	defs.h $(FRC)

ctype.o:	ctype.h
		CC=$(CC) AS=$(AS) CFLAGS="$(CFLAGS)" sh ./:fix ctype

blok.o fault.o main.o stak.o:	brkincr.h


cwd.o:	mac.h

cmd.o macro.o main.o msg.o name.o word.o xec.o:	sym.h

main.o:	timeout.h


msg.o:		msg.c $(FRC)
			CC=$(CC) AS=$(AS) CFLAGS="$(CFLAGS)" sh ./:fix msg

test:
	  rtest $(TESTDIR)/sh

install:  all
	mv /bin/sh /bin/osh
	cp sh /bin/sh
	strip /bin/sh
	/etc/chown bin,bin /bin/sh
	chmod o-w,g+w /bin/sh

clean:
	  -rm -f *.o sh

clobber:  clean
	  -rm -f $(TESTDIR)/sh
	  -rm -f $(ROOT)/bin/OLDrsh

FRC:

pp:
	pp -tsh makefile *.h *.c | dcan
