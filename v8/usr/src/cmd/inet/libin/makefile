INSTALLDIR=/usr/lib
CFLAGS=-g -I ../h
ALL=libin.a
OBJS=tcp_lib.o udp_lib.o\
	in_service.o in_address.o in_host.o in_subrs.o in_ntoa.o in_ntoh.o

all	:	${ALL}


libin.a	:	${OBJS}
		ar ru libin.a ${OBJS}

in_ntoh.o:	in_ntoh.s
		cc -c in_ntoh.s

${OBJS}:	../h/config.h

install	:	all
		cp ${ALL} ${INSTALLDIR}
		( cd ${INSTALLDIR} ; ranlib ${ALL} )

clean	:
		rm -f ${ALL} *.o


nlibin.a:	${NOBJS}
		ar ru libin.a ${NOBJS}

