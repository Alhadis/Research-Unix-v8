# If you don't want to plot, take out the -Dplot
CFLAGS=-Dplot -Od2
prof:	prof.c
	$(CC) $(CFLAGS) prof.c -o prof

install: prof
	strip prof
	cp prof $(DESTDIR)/usr/bin

clean :
	rm -f *.o prof
