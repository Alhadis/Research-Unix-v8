.c.o:
	mcc -m -c $<

.c.s:
	mcc -S $<

loop: loop.c
	cc -o loop loop.c

diag: diag.o
	mcc -o diag -b 256 diag.o -lj

foo: foo.o
	mld -o foo -b 256 /usr/jerq/lib/l.o foo.o -lsys -lj /usr/jerq/lib/libc.a

romdiag: diag.s
	sed -f makerom.sed < diag.s > romdiag.s
	mcc -c romdiag.s
	mld -o romdiag -b 262144 romdiag.o -lj /usr/jerq/lib/libc.a

bits: romdiag
	strip romdiag
	dd if=romdiag of=bits bs=32 skip=1 count=256

prom:	bits
	push r70 bits /usr/rob/jerq/bits
	/usr/games/fortune
	r70 "cd /usr/rob/jerq/bits; dobits bits"
