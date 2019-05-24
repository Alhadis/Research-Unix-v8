all:	vismon vismon.m
vismon:	host-vismon.c
	cc -o vismon host-vismon.c
vismon.m:	dud-vismon.c
	3cc -o vismon.m dud-vismon.c
install:	vismon vismon.m
	rm /usr/dud/bin/vismon
	cp vismon /usr/dud/bin
	cp vismon.m /usr/dud/mbin/vismon.m
debug:	vismon vismon.m
	rm /usr/dud/bin/vismon
	cp vismon /usr/dud/bin
	cp vismon.m /usr/dud/mbin/vismon.m
clean:
	rm -f core *.o vismon vismon.m m.out
