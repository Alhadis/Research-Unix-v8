CFLAGS=-O -Dunix=1 -Dvax=1 -DFLEXNAMES -DMTIME

cpp :	cpp.o cpy.o rodata.o
	$(CC) $(CFLAGS) -o cpp rodata.o cpp.o cpy.o  

cpy.c : cpy.y
	yacc cpy.y
	:yyfix yyexca yyact yypact yypgo yyr1 yyr2 yychk yydef
	mv y.tab.c cpy.c
cpy.o :	cpy.c yylex.c
rodata.o : cpy.c
	$(CC) $(CFLAGS) -R -c rodata.c

clean :
	rm -f *.s *.o cpy.c rodata.c cpp y.tab.c

install :	cpp
	strip cpp
	mv cpp $(DESTDIR)/lib

test:
	rm -fr testdir
	mkdir testdir
	cp /lib/c2 /lib/ccom testdir
	make clean
	make cpp
	cp cpp testdir/cpp
	make clean
	make CFLAGS="${CFLAGS} -Btestdir/" cpp
	cp cpp testdir/cpp
	make clean
	make CFLAGS="${CFLAGS} -Btestdir/" cpp
	cmp cpp testdir/cpp
	rm -fr testdir
