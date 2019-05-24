OFILES=batch.o mccsymtab.o snetmaster.o snetcore.o snetproc.o wd.o kerncore.o kernmaster.o kernproc.o hostcore.o master.o hostmaster.o termmaster.o asm.o audit.o bpts.o coffsymtab.o core.o dtype.o ed8symtab.o expr.o format.o frame.o gram.o lib.o main.o memory.o parse.o phrase.o process.o srctext.o symbol.o symtab.o termcore.o termproc.o sigmask.o srcdir.o hostproc.o vaxoptab.o mac32optab.o
PRIFILES=asm.pri bpts.pri core.pri format.pri expr.pri frame.pri master.pri memory.pri phrase.pri process.pri srctext.pri symtab.pri
HFILES=asm.h bpts.h coff.h core.h dtype.h ed8.h expr.h format.h frame.h gram.h hostcore.h lib.h master.h memory.h mip.h op.h vaxoptab.h mac32optab.h parse.h phrase.h process.h sigmask.h srcdir.h srctext.h symbol.h symtab.h termcore.h univ.h y.tab.h
PUBFILES=asm.pub bpts.pub core.pub expr.pub format.pub frame.pub master.pub memory.pub phrase.pub process.pub srctext.pub symtab.pub

INC=-I/usr/jerq/include
CFLAGS=+E
CLEAVE=/usr/tac/bin/cleave
LIB=/usr/jerq/lib/libpads.a -ljobs -ldk

4pi:	$(PRIFILES) $(PUBFILES) $(HFILES) $(OFILES)
	rm -f pi 3pi 4pi
	cc -o 4pi $(OFILES) $(LIB)
	strip -s -g 4pi
	ln 4pi 3pi
	ln 4pi pi
	@beep

%.pri:	%.h
	$(CLEAVE) $(INC) $(CFLAGS) $*
	touch $*.pri

%.o:	%.c
	CFRONTOPTS=+E CPPOPTS=$(INC) /usr/tac/bin/CCG `pwd`/$*.c

gram.c:	gram.y
	yacc -d gram.y
	mv y.tab.c gram.c

clean:
	rm -f *.s *..s *junk* core jim* *t *.cppd

install:
	rm -f /usr/jerq/bin/4pi /usr/jerq/bin/3pi /usr/jerq/bin/pi
	cp 4pi /usr/jerq/bin
	chmod 775 /usr/jerq/bin/4pi
	ln /usr/jerq/bin/4pi /usr/jerq/bin/pi
	ln /usr/jerq/bin/4pi /usr/jerq/bin/3pi
