CFLAGS=-O -I/usr/blit/include
mas: mas0.o mas1.o mas2.o mas3.o mas4.o
	$(CC) $(CFLAGS) -i -o mas mas0.o mas1.o mas2.o mas3.o mas4.o
y.tab.h:	mas.yh
mas.yh:	y.tab.h
	- cmp -s y.tab.h mas.yh || cp y.tab.h mas.yh
mas1.o: mas.h mas.yh mas1.c
mas2.o: mas.h mas.yh mas2.c
mas3.o: mas.h mas3.c
mas4.o: mas.h mas.yh mas4.c ops.c

print:
	@pr mas.h /usr/blit/include/a.out.h mas0.y mas1.c mas2.c mas3.c mas4.c ops.c
