MCC=/usr/blit/bin/mcc
MLD=/usr/blit/bin/mld
CFILES=../libj/put.c ../libj/printf.c ../libj/comm.c ../libj/fopen.c\
	../libj/rdwr.c ../libj/data.c ../libj/get.c ../libj/popen.c\
	../libj/exit.c

all: jx sysint

jx: jx.c
	cc -g -o jx jx.c

.c.o:
	$(MCC) -c $<

sysint: sysint.c comm.h
	cc -g -o sysint sysint.c

comm.h: $(CFILES)
	grep "define._SYS" $(CFILES) | sed "s/.*://" > comm.h

print:
	@pr /usr/include/jerqio.h comm.h $(CFILES)

install:	jx sysint
	cp jx /usr/blit/bin
	cp sysint /usr/blit/lib

clean:
	rm -f core *.o hello sysint jx
