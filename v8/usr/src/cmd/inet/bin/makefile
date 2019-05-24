INSTALLDIR=/usr/inet/bin
CFLAGS=-g -I ../h
ALL=rsh rogin rcp netstat telnet

all	:	${ALL}


rsh	:	rsh.o
		cc ${CFLAGS} -o rsh rsh.o -lin

rogin	:	rogin.o
		cc ${CFLAGS} -o rogin rogin.o -lin

rcp	:	rcp.o
		cc ${CFLAGS} -o rcp rcp.o -lin

netstat	:	netstat.o
		cc ${CFLAGS} -o netstat netstat.o -lin

route	:	route.o
		cc ${CFLAGS} -o route route.o -lin

telnet	:	telnet.o
		cc ${CFLAGS} -o telnet telnet.o -lin

install	:	all
		( cd ${INSTALLDIR} ; rm -f ${ALL} ropy rlogin )
		cp ${ALL} ${INSTALLDIR}
		( cd ${INSTALLDIR} ; strip ${ALL} )
		( cd ${INSTALLDIR} ; /etc/chown root rsh rogin rcp telnet )
		( cd ${INSTALLDIR} ; chmod go-w,u+s  rsh rogin rcp telnet )
		( cd ${INSTALLDIR} ; chgrp sys  netstat )
		( cd ${INSTALLDIR} ; chmod go-w,g+s  netstat; ln rogin rlogin; ln rcp ropy)

clean	:
		rm -f ${ALL} *.o
