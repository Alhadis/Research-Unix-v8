jplot: libblit.a driver.o
	cc driver.o libblit.a -lm -o jplot
unplot: unplot.c
	ar x blit.c.a jcom.h
	cc unplot.c -o unplot
libblit.a: blit.c.a
	mkdir xplot
	cd xplot;ar x ../blit.c.a
	cd xplot;cc -c -O *.c
	cd xplot;ar rc libblit.a *.o
	mv xplot/libblit.a libblit.a
	ranlib libblit.a
	rm xplot/*
	rmdir xplot
jplot.m:	inblit.a
	mkdir xplot
	cd xplot;ar x ../inblit.a
	cd xplot; ar x ../blit.c.a jcom.h
	cd xplot;mcc *.c -lmj -o jplot.m
	mv xplot/jplot.m jplot.m
	rm xplot/*
	rmdir xplot
jplot.j:	inblit.a
	mkdir xplot
	cd xplot;ar x ../inblit.a
	cd xplot;ar x ../blit.c.a jcom.h
	cd xplot;mcc -j *.c -lmj -o jplot.j
	mv xplot/jplot.j jplot.j
	rm xplot/*
	rmdir xplot
install:	jplot.m jplot
	cp jplot.m /usr/blit/mbin
	cp jplot.j /usr/blit/mbin
	cp jplot /usr/blit/bin
clean:
	rm -f jplot.m jplot.j jplot *.o
	rm -fr xplot
