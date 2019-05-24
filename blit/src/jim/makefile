CFLAGS=-gO
CC=mcc
CFILES=main.c frame.c menu.c msgs.c string.c frame/deltext.c frame/frameop.c frame/instext.c frame/newframe.c frame/ptofchar.c
FILES=main.o frame.o menu.o msgs.o string.o frame/frame.a

xr.m:	$(FILES)
	cd frame; make
	mcc -o xr.m $(FILES)

xnr.m:	$(FILES) frame/frame.a
	/usr/pjw/nld/mld -M -d -r -o xnr.m /usr/blit/lib/notsolow.o $(FILES) -lmj -lj -lc

print:
	@pr frame/frame.h $(CFILES)

pp:
	@pp -t"Terminal Jim" r.h menu.h msgs.h frame/frame.h $(CFILES) | dcan

lint:
	lint -Dvoid=int -I/usr/blit/include $(CFILES) -lj

$(FILES): frame/frame.h /usr/blit/include/blit.h /usr/blit/include/jerqproc.h
main.o frame.o menu.o:	r.h
r.h:	frame/frame.h menu.h
.PRECIOUS:	frame.a

install:	xr.m
	mopt xr.m
	cp m.out /usr/blit/mbin/jim.m
	mstrip -gv /usr/blit/mbin/jim.m
	cd file; make install

clean:
	rm -f *.o xr.m core m.out
	cd file; make clean
	cd frame; make clean
