PDIR = proto
INCL = $(PDIR)
JINCL = /usr/jerq/include
SFLAGS =
CFLAGS = -I$(INCL) -I$(JINCL) $(SFLAGS) -O
PHEADERS = $(PDIR)/pconfig.h $(PDIR)/packets.h $(PDIR)/proto.h $(PDIR)/pstats.h
PFILES = $(PDIR)/pcheck.c $(PDIR)/pinit.c \
	 $(PDIR)/precv.c $(PDIR)/psend.c $(PDIR)/ptimeout.c
POBJS = pcheck.o pinit.o precv.o psend.o ptimeout.o

mpx:	mpx.o queue.o $(POBJS)
	$(CC) $(LDFLAGS) -o mpx mpx.o queue.o $(POBJS)

tmpx:	tmpx.o queue.o $(POBJS)
	$(CC) $(LDFLAGS) -o tmpx tmpx.o queue.o $(POBJS)

tmpx.o:	mpx.c
	-mv mpx.o smpx.o
	$(CC) $(CFLAGS) -DTRACING -c mpx.c && mv mpx.o tmpx.o
	-mv smpx.o mpx.o

mpx.o:		queue.h msgs.h $(PDIR)/proto.h $(PDIR)/packets.h $(PDIR)/pstats.h
queue.o:	queue.h

lint:
	lint -I$(INCL) mpx.c queue.c $(PDIR)/*.c

pcheck.o:	$(PDIR)/pcheck.c
		$(CC) $(CFLAGS) -c $(PDIR)/pcheck.c
pcntrl.o:	$(PDIR)/pcntrl.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/pcntrl.c
pinit.o:	$(PDIR)/pinit.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/pinit.c
precv.o:	$(PDIR)/precv.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/precv.c
psend.o:	$(PDIR)/psend.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/psend.c
ptimeout.o:	$(PDIR)/ptimeout.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/ptimeout.c

print:
	pr makefile *.h *.c
