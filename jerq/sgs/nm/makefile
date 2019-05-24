CFLAGS=-O -c
ARCH=AR32WR
#	Set ARFORMAT to "PORTAR" for portable archive format (post 5.0)
ARFORMAT=PORTAR
#	Set FLEX to -DFLEXNAMES for flexnames (post 6.0)
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(ARFORMAT) -DUNIX $(FLEX)

OBJECTS = main.o process.o prosym.o decotype.o fcns.o setflags.o 

all:	nm
	$(CC) -o nm $(OBJECTS) ../libld/libld.a

nm:	$(OBJECTS)
	$(CC) -o nm $(OBJECTS) ../libld/libld.a

%.o:	%.c
	$(CC) $(CFLAGS) -I../inc $(DEFLIST) $%.c

$(OBJECTS):	decotype.h defs.h prosym.h

install:	nm
	cp nm /usr/jerq/bin/3nm
	strip /usr/jerq/bin/3nm

clean:
	rm -f 3nm nm *.o core
