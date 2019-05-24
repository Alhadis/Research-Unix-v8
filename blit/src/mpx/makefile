LIBS =
PDIR = proto
INCL = $(PDIR)
JINCL = /usr/blit/include
SFLAGS =
CFLAGS = -I$(INCL) -I$(JINCL) $(SFLAGS)
PHEADERS = $(PDIR)/pconfig.h $(PDIR)/packets.h $(PDIR)/proto.h $(PDIR)/pstats.h
PFILES = $(PDIR)/pcheck.c $(PDIR)/pinit.c \
	 $(PDIR)/precv.c $(PDIR)/psend.c $(PDIR)/ptimeout.c
POBJS = pcheck.o pinit.o precv.o psend.o ptimeout.o

mpx:	mpx.o $(POBJS)
	$(CC) $(LDFLAGS) -o mpx mpx.o $(POBJS) $(LIBS)

smpx:	smpx.o $(PFILES)
	$(CC) $(CFLAGS) -DPSTATISTICS -DPSTATSDESC -c $(PFILES)
	$(CC) $(LDFLAGS) -o smpx smpx.o $(POBJS) $(LIBS)
	-rm -f $(POBJS)

ssmpx:	smpx.o $(PFILES)
	$(CC) $(CFLAGS) -DEBUG -c $(PFILES)
	$(CC) $(LDFLAGS) -o ssmpx smpx.o $(POBJS) $(LIBS)
	-rm -f $(POBJS)

tmpx:	tmpx.o $(PFILES)
	$(CC) $(CFLAGS) -DEBUG -c $(PFILES)
	$(CC) $(LDFLAGS) -o tmpx tmpx.o $(POBJS) $(LIBS)
	-rm -f $(POBJS)

smpx.o:	mpx.c
	-mv mpx.o savempx.o
	$(CC) $(CFLAGS) -DPSTATISTICS -DMPXSTATS -DPSTATSDESC -c mpx.c && mv mpx.o smpx.o
	-mv savempx.o mpx.o

tmpx.o:	mpx.c
	-mv mpx.o smpx.o
	$(CC) $(CFLAGS) -DTRACING -DEBUG -c mpx.c && mv mpx.o tmpx.o
	-mv smpx.o mpx.o

mpx.o:		msgs.h $(PHEADERS)

pcheck.o:	$(PDIR)/pcheck.c
		$(CC) $(CFLAGS) -c $(PDIR)/pcheck.c
pinit.o:	$(PDIR)/pinit.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/pinit.c
precv.o:	$(PDIR)/precv.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/precv.c
psend.o:	$(PDIR)/psend.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/psend.c
ptimeout.o:	$(PDIR)/ptimeout.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/ptimeout.c

print:
	pr *.h *.c

pp:
	pp -tmpx *.h *.c proto/*.? | dcan

lint:
	lint -I$(INCL) -I$(JINCL) $(SFLAGS) -DTRACING mpx.c $(PDIR)/*.c

clean:
	rm -f *.o mpx tmpx
	cd term; make clean
install:	mpx
	cp mpx /usr/blit/bin
	cd term; make install
