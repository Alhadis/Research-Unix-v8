I8SEND = uucpinter
CFLAGS = -O
LIBES =
FILES1 =  Makefile efl.1 defs tokens main.c init.c tailor.c\
		gram.head gram.dcl gram.expr gram.exec\
		lex.l fixuplex symtab.c\
		dcl.c addr.c struct.c field.c misc.c alloc.c mk.c
FILES2 =  exec.c blklab.c simple.c print.c temp.c io.c error.c\
		pass2.c icfile.c free.c dclgen.c namgen.c
FILES = $(FILES1) $(FILES2)
P = detab 0 6 12 18 24 30 36 42 48 54 60 66 72 | und -3 | opr -r2

OBJECTS = main.o init.o tailor.o gram.o lex.o symtab.o\
	dcl.o addr.o struct.o field.o blklab.o\
	mk.o misc.o alloc.o print.o simple.o exec.o temp.o io.o error.o\
	pass2.o icfile.o free.o dclgen.o namgen.o

a.out : $(OBJECTS)
	@echo LOAD
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBES)
	@size

$(OBJECTS) :  defs
lex.o init.o : tokdefs

# gram.c can no longer be made on a pdp11 because of yacc limits
#gram.c:	gram.head gram.dcl gram.expr gram.exec tokdefs
#	( sed <tokdefs "s/#define/%token/" ;\
#		cat gram.head gram.dcl gram.expr gram.exec) >gram.in
#	$(YACC) $(YFLAGS) gram.in
#	mv y.tab.c gram.c
#	rm gram.in

lex.c: fixuplex
	lex lex.l
	fixuplex
	mv lex.yy.c lex.c

tokdefs: tokens
	grep -n . <tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" >tokdefs

clean:
	rm -f a.out *.o  lex.c tokdefs efl[cde].* nohup.out

printall:
	-pr $(FILES1) | $P
	-pr $(FILES2) | $P
	touch print

print:  $(FILES)
	-pr $? | $P
	touch print

arch:   /usr/sif/efl/arch

/usr/sif/efl/arch :  $(FILES)
	@ar uv /usr/sif/efl/arch $?
	@wc /usr/sif/efl/arch

lint: a.out
	lint -p *.c -lS

install: a.out
	strip a.out
	cp a.out /usr/bin/efl

save:
	-fsend -c -u efl /usr/sif/efl/arch

man:	/sys/man/man1/efl.1
	man efl

/sys/man/man1/efl.1:	efl.1
	cp efl.1 /sys/man/man1

test:	a.out
	a.out "system=gcos" /usr/sif/efl/test/Hard.e >z1 2>z2
	cmp z1 /usr/sif/efl/test/Hard.out
	a.out "system=gcos" /usr/sif/efl/test/Band.e >z1 2>z2
	cmp z1 /usr/sif/efl/test/Band.out
	a.out "system=gcos" /usr/sif/efl/test/Buram.e >z1 2>z2
	cmp z1 /usr/sif/efl/test/Buram.out
	a.out "system=gcos" /usr/sif/efl/test/Dgl.e >z1 2>z2
	cmp z1 /usr/sif/efl/test/Dgl.out
	rm z1 z2
	@echo TEST OK

gcosall:	$(FILES) tokdefs gram.c lex.c
	-fsend -c -u efl $(FILES) tokdefs gram.c lex.c
	touch gcos

gcos:	$(FILES) tokdefs gram.c lex.c
	-fsend -c -u efl $?
	touch gcos

interall:
	$(I8SEND) $(FILES) gram.c lex.c
	touch interdata

interdata:  $(FILES) gram.c lex.c
	$(I8SEND) $?
	touch interdata

tape: $(FILES) gram.c lex.c a.out
	tp mr $(FILES) gram.c lex.c a.out

src: $(FILES) tokdefs gram.c
	cp $? /crp/src/cmd/efl
	touch src
