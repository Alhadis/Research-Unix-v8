FROMMMDF=frommmdf.o from822.o header.o ../common/subrs.o 
TOMMDF=tommdf.o to822.o header.o ../common/subrs.o 
CFLAGS=-g -I../common

all: frommmdf tommdf

frommmdf: ${FROMMMDF}
	cc ${FROMMMDF} -o frommmdf

tommdf: ${TOMMDF}
	cc ${TOMMDF} -o tommdf

../common/subrs.o: ../common/subrs.c ../common/mail.h
	${CC} -c ${CFLAGS} ../common/subrs.c
	mv subrs.o ../common/subrs.o

frommmdf.o: ../common/mail.h header.h
from822.o: ../common/mail.h header.h
tommdf.o: ../common/mail.h header.h
to822.o: ../common/mail.h header.h

cyntax:
	cyntax -I../common frommmdf.c from822.c header.c ../common/subrs.c
	cyntax -I../common tommdf.c to822.c header.c ../common/subrs.c 

install: frommmdf tommdf
	cp frommmdf tommdf /usr/lib/upas
	strip /usr/lib/upas/frommmdf /usr/lib/upas/tommdf

clean:
	rm -f from822 frommmdf tommdf *.o ../common/*.o

