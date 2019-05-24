CFLAGS=
MCC=/usr/blit/bin/mcc -DM68000
MOPT=/usr/blit/bin/mopt
MSTRIP=/usr/blit/bin/mstrip
DCC=/usr/jerq/bin/3cc
CC=cc

MONOBJ=calldaemon.o sname.o sampleload.o scanmail.o utmp.o icon.o from.o nlist.o
MONSRC=sysmon.c calldaemon.c sname.c sampleload.c scanmail.c utmp.c icon.c from.c
DAEOBJ=sampleload.o sname.o daemon.o ntdkmgr.o generate.o scanmail.o
MONDEP=sysmon.c load.h defs.h msg.h scanmail.h
DISPDEP=jerqmon.c

all: sysmond sysmon sysmon.m lucamon.m
install: install.bin install.mbin install.lib
ship: ship.bin ship.mbin ship.lib

#
#  create the daemon
#
sysmond: ${DAEOBJ}
	${CC} ${CFLAGS} ${DAEOBJ} -o sysmond -ldk -ljobs

sampleload.o: defs.h load.h
sname.o: defs.h
calldaemon.o: defs.h
daemon.o: defs.h
ntdkmgr.o:
generate.o: defs.h load.h scanmail.h msg.h
scanmail.o: scanmail.h defs.h

#
#  create the communications window routine
#

#
#  create the host piece (same for 68000 or MAC32)
#
sysmon: sysmon.o ${MONOBJ}
	${CC} ${CFLAGS} -o sysmon sysmon.o ${MONOBJ} -ldk

sysmon.o: ${MONDEP}
	${CC} ${CFLAGS} -c sysmon.c

cyn.bin: ${MONSRC}
	cyntax ${MONSRC}

#
#  create the 68000 version
#
#
# sysmon.m.68: ${DISPDEP}
# 	${MCC} ${CFLAGS} -o sysmon.m.68 jerqmon.c
# lucamon.m.68: ${DISPDEP}
# 	${MCC} ${CFLAGS} -DLUCA -o lucamon.m.68 jerqmon.c

#
#  create the MAC 32 version
#
sysmon.m: ${DISPDEP}
	-${DCC} ${CFLAGS} -o sysmon.m jerqmon.c
lucamon.m: ${DISPDEP}
	-${DCC} ${CFLAGS} -DLUCA -o lucamon.m jerqmon.c

#
#  install things
#
install.bin:	sysmon
	-rm -f /usr/jerq/bin/sysmon
	cp sysmon /usr/jerq/bin
	/etc/chown bin,sys /usr/jerq/bin/sysmon
	chmod 2755 /usr/jerq/bin/sysmon
	/bin/rm -f /usr/jerq/bin/vismon
	ln /usr/jerq/bin/sysmon /usr/jerq/bin/vismon

ship.bin:
	ship /usr/jerq/bin/sysmon /usr/jerq/bin/vismon

install.mbin: sysmon.m lucamon.m
	cp sysmon.m /usr/jerq/mbin/sysmon.m
	cp lucamon.m /usr/jerq/mbin/lucamon.m

ship.mbin:
	ship /usr/jerq/mbin/sysmon.m /usr/jerq/mbin/lucamon.m

install.lib:
	cp sysmond /usr/jerq/lib
	/etc/chown root,sys /usr/jerq/lib/sysmond
	chmod 4755 /usr/jerq/lib/sysmond

ship.lib:
	ship /usr/jerq/lib/sysmond

#
#  misc
#
clean:
	rm -f core *.o sysmon *.m.68 *.m.32 *.out sysmond vismon lucamon.m sysmon.m

smtest: smtest.o scanmail.o
	${CC} smtest.o scanmail.o -o smtest

smtest.o: defs.h scanmail.h
