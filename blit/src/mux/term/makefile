JINCL = /usr/blit/include
CC = mcc
CFLAGS = -g -O -m -DMPXTERM -I$(PDIR) -DBlit
AS = /usr/blit/bin/mas

JERQPROC = $(JINCL)/jerqproc.h
JHEADERS = $(JINCL)/layer.h $(JINCL)/jerq.h $(JERQPROC)

PDIR = ../proto
PHEADERS = $(PDIR)/packets.h $(PDIR)/proto.h $(PDIR)/pstats.h $(PDIR)/pconfig.h
PFILES = $(PDIR)/pcheck.c $(PDIR)/pinit.c \
	 $(PDIR)/precv.c $(PDIR)/psend.c $(PDIR)/ptimeout.c
POBJS = pcheck.o pinit.o precv.o psend.o ptimeout.o

TFILES = boot.c comm.c control.c demux.c task.c trap.c user.c window.c
TOBJS =  boot.o comm.o control.o demux.o task.o trap.o user.o window.o

OBJS = $(TOBJS) $(POBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(CC) -c $<

muxterm: & Pl.o $(OBJS) /usr/blit/lib/libsys.a /usr/blit/lib/liblayer.a /usr/blit/lib/libj.a frame/frame.a rom.o
	mld -o muxterm -b 256 Pl.o $(OBJS) frame/frame.a rom.o\
		 -llayer -lsys -lj -lc 
#	mnm -g muxterm | grep Sys

frame/frame.a:
	cd frame; make
install:	muxterm
	cp muxterm /usr/blit/lib
	mstrip -g /usr/blit/lib/muxterm
Muxterm: & Pl.o norompcheck.o $(OBJS) /usr/blit/lib/libsys.a /usr/blit/lib/liblayer.a /usr/blit/lib/libj.a frame/frame.a 
	mld -o Muxterm -b 256 Pl.o norompcheck.o $(OBJS) frame/frame.a \
		 -llayer -lsys -lj -lc 
#	mnm -g muxterm | grep Sys
#	: Should be 0106

rom.o:	rom.s
rom.s:	romroutines
	sort romroutines | sed 's/\(........\).*/\1/' | join -j2 3 - romterm.nm | sed -f romsed.f > rom.s


goo.s:	makelocore /usr/blit/include/mpx.h $(JERQPROC) Pl.template
	makelocore goo.s /usr/blit/include/mpx.h Pl.template $(AS)

Pl.o:	goo.s
	$(CC) -S procsize.c
	sed 1s/999/`sed -n '/short/s/.*	//p' < procsize.s`/ < goo.s > Pl.s
	$(CC) -c Pl.s

print:
	@pr /usr/blit/include/mpx.h $(JHEADERS) $(JINCL)/jioctl.h $(JINCL)/queue.h ../msgs.h \
		Pl.template $(TFILES)

pp:
	pp -kjerq -tmuxterm /usr/blit/include/mpx.h $(JHEADERS) $(JINCL)/jioctl.h $(JINCL)/queue.h ../msgs.h \
		Pl.template $(TFILES) | dcan

lint:
	lint -I/usr/blit/include $(CFLAGS) -Dmc68000 -Uvax -Dvoid=int $(TFILES) $(PFILES) -lj -llayer 2>&1 | buffer

user.o:		$(JHEADERS)

window.o:	$(JHEADERS) $(JINCL)/tty.h $(JINCL)/queue.h frame/frame.h

boot.o:		$(JHEADERS)
comm.o:		$(JHEADERS) $(JINCL)/queue.h
control.o:	$(JHEADERS) $(JINCL)/queue.h $(PHEADERS) ../msgs.h
demux.o:	$(JHEADERS) $(JINCL)/jioctl.h $(JINCL)/queue.h $(PHEADERS) ../msgs.h
task.o:		$(JHEADERS)
trap.o:		$(JHEADERS)

pcheck.o:	$(PDIR)/pcheck.c
		$(CC) $(CFLAGS) -DROMCRC -c $(PDIR)/pcheck.c
norompcheck.o:	$(PDIR)/pcheck.c
		$(CC) $(CFLAGS) -c $(PDIR)/pcheck.c
		mv pcheck.o norompcheck.o
pinit.o:	$(PDIR)/pinit.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/pinit.c
precv.o:	$(PDIR)/precv.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/precv.c
psend.o:	$(PDIR)/psend.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/psend.c
ptimeout.o:	$(PDIR)/ptimeout.c $(PHEADERS)
		$(CC) $(CFLAGS) -c $(PDIR)/ptimeout.c
#gcalloc.o:	gcalloc.c
#	$(CC) $(CFLAGS) -S gcalloc.c
#	sed -f sed.f < gcalloc.s > goo
#	mv goo gcalloc.s
#	$(CC) -c gcalloc.s
#	rm gcalloc.s
texture.o:	texture.s
	/lib/cpp -P texture.s > goo.s
	mcc -c goo.s && mv goo.o texture.o

clean:
	rm -f *.o *.hup* goo.s core muxterm
	cd frame; make clean
