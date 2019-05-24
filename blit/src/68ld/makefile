BIN	= /usr/blit/bin
CFLAGS	= -I/usr/blit/include
RM	= /bin/rm -f

.c.o:
		$(CC) $(CFLAGS) -c $<

68ld:		68ld.o proto.o pcheck.o
		$(CC) $(LFLAGS) -o 68ld 68ld.o proto.o pcheck.o

68ld.o:		proto.h $(FRC)
proto.o:	proto.h $(FRC)

install:	68ld
		cp 68ld $(BIN)

clean:
	$(RM) *.o* 68ld

clobber:	clean
		$(RM) 68ld

lint:
		lint $(CFLAGS) *.c

print:
		@pr -n3 -l66 -w132 files makefile *.h *.c

pp:
		pp -t68ld makefile *.h *.c *.cm *.sm | dcan

cpio:
		cpio -oc <files >68ld.cpio

FRC:
