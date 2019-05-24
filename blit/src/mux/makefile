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

mux:	mux.o $(POBJS)
	$(CC) $(LDFLAGS) -o mux mux.o $(POBJS) $(LIBS)

smux:	smux.o $(PFILES)
	$(CC) $(CFLAGS) -DPSTATISTICS -DPSTATSDESC -c $(PFILES)
	$(CC) $(LDFLAGS) -o smux smux.o $(POBJS) $(LIBS)
	-rm -f $(POBJS)

ssmux:	smux.o $(PFILES)
	$(CC) $(CFLAGS) -DEBUG -c $(PFILES)
	$(CC) $(LDFLAGS) -o ssmux smux.o $(POBJS) $(LIBS)
	-rm -f $(POBJS)

tmux:	tmux.o $(PFILES)
	$(CC) $(CFLAGS) -DEBUG -c $(PFILES)
	$(CC) $(LDFLAGS) -o tmux tmux.o $(POBJS) $(LIBS)
	-rm -f $(POBJS)

smux.o:	mux.c
	-mv mux.o savemux.o
	$(CC) $(CFLAGS) -DPSTATISTICS -DMPXSTATS -DPSTATSDESC -c mux.c && mv mux.o smux.o
	-mv savemux.o mux.o

tmux.o:	mux.c
	-mv mux.o smux.o
	$(CC) $(CFLAGS) -DTRACING -DEBUG -c mux.c && mv mux.o tmux.o
	-mv smux.o mux.o

mux.o:		msgs.h /usr/blit/include/tty.h $(PHEADERS)

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
	pp -tmux *.h *.c proto/*.? | dcan

lint:
	lint -I$(INCL) -I$(JINCL) $(SFLAGS) -DTRACING mux.c $(PDIR)/*.c

clean:
	rm -f *.o mux tmux
	cd term; make clean

install:	mux
	cp mux /usr/blit/bin
	cd term; make install
