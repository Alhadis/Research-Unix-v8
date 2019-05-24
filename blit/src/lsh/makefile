term:	term.c
	mcc -g term.c
	mopt a.out|grep "%"
	rm a.out
	mv m.out term

host:	host.c
	cc -g -o host host.c 

install: host term
	cp term /usr/jerq/mbin/lsh.m
	cp host /usr/jerq/lib/lsh
