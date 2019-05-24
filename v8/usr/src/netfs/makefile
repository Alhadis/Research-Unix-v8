YFLAGS=-D
CFLAGS=-g
LDFLAGS=-g
SOURCE=dkstuff.c fserv.h main.c ps.y perm.c start.c sub.c sys.c work.c \
	setup.c
OBJ=main.o dkstuff.o ps.o perm.o sys.o start.o sub.o work.o
all: setup fshare unmount
fshare:	$(OBJ) profit.o
	$(CC) -o fshare $(LDFLAGS) $(OBJ) profit.o -ldk -ljobs
list:	$(SOURCE) makefile
	-prcan $?
	touch list
$(OBJ):	fserv.h
setup:	setup.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o setup setup.c setjmp.s -ldk
unmount: unmount.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o unmount unmount.c
ptest:	ptest.o perm.o ps.o
	cc -o ptest ptest.o perm.o ps.o
ps.c:	ps.y
	yacc $(FLAGS) ps.y
	sed < y.tab.c > ps.c '/^# line/d'
clean:
	rm -f *.o fshare setup unmount ps.c
install:
	strip fshare setup unmount
	cp fshare setup unmount /usr/net
	cp share.go setup.go teardown magic pscript /usr/net
