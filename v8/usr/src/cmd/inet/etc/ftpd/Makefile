#	@(#)Makefile	4.2	6/12/83
#
ALL=	ftpd.o ftpcmd.o glob.o vers.o mkdir.o rmdir.o
DESTDIR=/usr/inet
CFLAGS=-I../../h

ftpd:	${ALL}
	${CC} -o ftpd ${ALL} -lin

vers.o:	ftpd.c ftpcmd.y
	sh newvers.sh
	${CC} -c vers.c

install:
	cp ftpd ${DESTDIR}/etc/ftpd
	strip ftpd
	chown bin,bin ${DESTDIR}/etc/ftpd

clean:
	rm -f ftpd *.o *.s errs core a.out t.? yacc.* y.tab.c
