CFLAGS=-g -I ../h
INSTALLDIR=/usr/inet/etc
ALL=ipconfig tcpconfig rogind rshd route\
	routed dkipconfig telnetd interlan udpconfig

all	:	${ALL}

ipconfig :	ipconfig.o
		cc ${CFLAGS} -o ipconfig ipconfig.o -lin

dipconfig :	dipconfig.o
		cc ${CFLAGS} -o dipconfig dipconfig.o -lin

nipconfig :	nipconfig.o
		cc ${CFLAGS} -o nipconfig nipconfig.o -lin

dkipconfig :	dkipconfig.o
		cc ${CFLAGS} -o dkipconfig dkipconfig.o -ldk -lin -ljobs

tcpconfig :	tcpconfig.o
		cc ${CFLAGS} -o tcpconfig tcpconfig.o

rogind	:	rogind.o
		cc ${CFLAGS} -o rogind rogind.o -lin

rshd	:	rshd.o
		cc ${CFLAGS} -o rshd rshd.o -lin

ftpd	:	ftpd.o
		cc ${CFLAGS} -o ftpd ftpd.o -lin

routed	:	routed.o
		cc ${CFLAGS} -o routed routed.o -lin

dkslip	:	dkslip.o
		cc ${CFLAGS} -o dkslip dkslip.o -ldk

route	:	route.o
		cc ${CFLAGS} -o route route.o -lin

telnetd	:	telnetd.o pt.o whoami.o
		cc ${CFLAGS} -o telnetd telnetd.o pt.o whoami.o -lin

install	:	all
		( cd ${INSTALLDIR} ; rm -f ${ALL} )
		cp ${ALL} ${INSTALLDIR}
		( cd ${INSTALLDIR} ; strip ${ALL} )
		( cd ${INSTALLDIR} ; /etc/chown daemon dkipconfig )
		( cd ${INSTALLDIR} ; chmod go-w,u+s dkipconfig )

clean	:
		rm -f ${ALL} *.o
