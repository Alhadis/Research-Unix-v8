RSRC= print.c semaphore.c\
	../common/tmpfile.c ../common/lock.c ../common/subrs.c
ROBJ= print.o semaphore.o\
	../common/tmpfile.o ../common/lock.o ../common/subrs.o
CFLAGS=-g -I../common

all: print

print: ${ROBJ}
	${CC} ${ROBJ} ${CFLAGS} -o print

print.o: ../common/mail.h ../common/letter.h ../common/string.h

../common/lock.o: ../common/lock.c ../common/string.h
	cd ../common; ${CC} -c ${CFLAGS} lock.c
../common/tmpfile.o: ../common/tmpfile.c ../common/mail.h ../common/letter.h
	cd ../common; ${CC} -c ${CFLAGS} tmpfile.c
../common/subrs.o: ../common/subrs.c ../common/string.h ../common/mail.h
	cd ../common; ${CC} -c ${CFLAGS} subrs.c

prcan:
	prcan ${RSRC}

clean:
	rm -f print *.o ../common.*.o

cyntax:
	cyntax -I../common ${RSRC}

install:	print
	cp print /usr/lib/upas/printmail
