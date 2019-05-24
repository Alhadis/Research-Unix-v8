LFLAGS = -hcp

DEST = /usr/bin
ETC = /etc

LIB = alloc.o args.o chk.o crypt.o data.o ftw.o fullname.o \
	gid.o header.o mkcsum.o mkdir.o package.o path.o pwd.o \
	tmpname.o transname.o uid.o

LIBSRC = alloc.c args.c chk.c crypt.c data.c ftw.c fullname.c \
	gid.c header.c mkcsum.c mkdir.c package.c path.c pwd.c \
	tmpname.c transname.c uid.c

all:	seal unseal mkpkg inspkg asdrcv

seal:	seal.o lib.a
	cc $(CFLAGS) -o seal seal.o lib.a

unseal:	unseal.o lib.a
	cc $(CFLAGS) -o unseal unseal.o lib.a

mkpkg:  mkpkg.o lib.a
	cc $(CFLAGS) -o mkpkg mkpkg.o lib.a

inspkg:	inspkg.o lib.a
	cc $(CFLAGS) -o inspkg inspkg.o lib.a

asdrcv:	asdrcv.o lib.a
	cc $(CFLAGS) -o asdrcv asdrcv.o lib.a

lib.a:	$(LIB)
	rm -f lib.a
	ar qc lib.a $(LIB)
	ranlib lib.a

lint:
	lint $(LFLAGS) mkpkg.c $(LIBSRC)
	lint $(LFLAGS) inspkg.c $(LIBSRC)
	lint $(LFLAGS) seal.c $(LIBSRC)
	lint $(LFLAGS) unseal.c $(LIBSRC)
	lint $(LFLAGS) asdrcv.c $(LIBSRC)

alloc.o args.o data.o fullname.o gid.o: asd.h ar.h
header.o inspkg.o mkdir.o: asd.h ar.h
mkpkg.o package.o path.o pwd.o seal.o tmpname.o: asd.h ar.h
transname.o uid.o unseal.o: asd.h ar.h
ftw.o mkdir.o package.c: ftw.h
mkcsum.o seal.o unseal.o: seal.h

clean:
	rm -f lib.a asdrcv mkpkg inspkg seal unseal *.o

install: all
	strip seal unseal mkpkg inspkg asdrcv
	mv seal unseal mkpkg inspkg ${DEST}
	/etc/chown bin,bin ${DEST}/seal ${DEST}/unseal ${DEST}/mkpkg ${DEST}/inspkg
	chmod 775 ${DEST}/seal ${DEST}/unseal ${DEST}/mkpkg ${DEST}/inspkg
	cp asdrcv ${ETC}/asd
	/etc/chown root ${ETC}/asd/asdrcv
	chmod og-w,u+s ${ETC}/asd/asdrcv
