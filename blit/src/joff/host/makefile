CFLAGS=
YFLAGS= 
a.out: 	stabread.o names.o environ.o alloc.o stabsearch.o main.o joff.o \
	jerq.o format.o snap.o stack.o gram.o context.o stabhelp.o traps.o \
	menu.o menus.o scratch.o expr.o chase.o srctext.o zdc.o \
	graphics.o functions.o
	cc *.o

oh:
	echo "#########" >>makefile
	oh *.h ../*.h >>makefile

lint:	joffhost
	lint *.c|tee lint.out

bin:
	cp /usr/tac/joff/bin/joffhost /usr/tac/joff/bin/joffhost$$$$
	cp a.out /usr/tac/joff/bin/joffhost

clean:	
	rm -f *.o a.out	

print:
	@pr -n *.h *.c *.y

lint.out: a.out
	lint *.c >lint.out

e:	egram.o alloc.o names.o environ.o expr.o
	cc -o e egram.o alloc.o names.o environ.o expr.o
	e

menus.c: ../menus.c
	cp ../menus.c menus.c

menus.o: ../menu.h
#########
stab.h :a.out.h 
alloc.o chase.o context.o environ.o expr.o format.o functions.o gram.o graphics.o jerq.o joff.o main.o menu.o menus.o scratch.o snap.o srctext.o stabhelp.o stabread.o stabsearch.o stack.o traps.o y.tab.o zdc.o :common.h 
chase.o expr.o gram.o y.tab.o traps.c :expr.h 
chase.o context.o expr.o joff.o menu.o srctext.o stack.o traps.o :frame.h 
context.o stab.h :mcc.h 
names.o stab.h :names.h 
stab.h :sdb.h 
main.o snap.o stabhelp.o stabread.o stabsearch.o user.h :stab.h 
chase.o context.o expr.o functions.o gram.o joff.o menu.o srctext.o stack.o traps.o y.tab.o :user.h 
stack.o traps.o :../isp.h 
menu.o menus.o scratch.o traps.o :../menu.h 
common.h :../protocol.h 
context.o expr.o gram.o joff.o menu.o srctext.o stack.o traps.o y.tab.o :../traps.h 
