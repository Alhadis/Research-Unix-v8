SSRC=	main.c rewrite.c notify.c send.c log.c okrmt.c
SOBJ=	main.o rewrite.o notify.o send.o log.o okrmt.o\
	../common/lock.o ../common/tmpfile.o ../common/subrs.o

CFLAGS=-g -I../common

all: send

send: ${SOBJ}
	${CC} ${SOBJ} ${CFLAGS} -o send

delivery.o: ../common/mail.h
log.o: ../common/mail.h
rewrite.o: ../common/mail.h ../common/string.h
drewrite.o: ../common/mail.h ../common/string.h rewrite.c
	${CC} -c ${CFLAGS} -DDEBUG rewrite.c
	mv rewrite.o drewrite.o
send.o: ../common/mail.h ../common/letter.h ../common/string.h
notify.o: ../common/string.h
okrmt.o:

../common/lock.o: ../common/lock.c ../common/string.h
	${CC} -c ${CFLAGS} ../common/lock.c
	mv lock.o ../common/lock.o
../common/tmpfile.o: ../common/tmpfile.c ../common/mail.h ../common/letter.h
	${CC} -c ${CFLAGS} ../common/tmpfile.c
	mv tmpfile.o ../common/tmpfile.o
../common/subrs.o: ../common/subrs.c ../common/string.h ../common/mail.h
	${CC} -c ${CFLAGS} ../common/subrs.c
	mv subrs.o ../common/subrs.o

prcan:
	prcan ${SSRC}

clean:
	rm -f send *.o ../common/*.o

cyntax:
	cyntax -I../common ${SSRC}

install:	send
	cp send /bin/mail
	chown root /bin/mail
	chmod 4755 /bin/mail
	rm -f /bin/rmail
	ln /bin/mail /bin/rmail

rtest: rtest.o drewrite.o okrmt.o ../common/subrs.o
	cc -o rtest rtest.o drewrite.o okrmt.o ../common/subrs.o
